#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_def.h"
#include "modbus_master_driver.h"
#include "cfg_mgr.h"
#include "list.h"
#include "modbus_master.h"
#include "modbus_tcp_master.h"
#include "modbus_rtu_master.h"
#include "modbus_util.h"
#include "channel_manager.h"
#include "evloop_timer.h"
#include "trace.h"
#include "time_util.h"
#include "app_init_completion.h"
#include "bit_util.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
#define APP_MODBUS_MAXTER_BOUNCE_BUFFER_SIZE          256

typedef struct
{
  struct list_head              le;
  modbus_master_driver_type_t   mb_type;
  ModbusMasterCTX*              ctx;
  modbus_register_list_t        reg_map;
  int                           current_request;
  int                           num_reqs;

  uint32_t                      schedule_finish_delay;
  uint32_t                      inter_request_delay;
  uint32_t                      timeout;

  unsigned long                 req_start_time;

  evloop_timer_t                wait_timer;
  evloop_timer_t                transaction_timer;

  modbus_master_driver_request_config_t*   request_schedule;

  uint32_t      n_request;
  uint32_t      n_response;
  uint32_t      n_rsp_timeout;
  uint32_t      min_transaction_time;
  uint32_t      max_transaction_time;
  uint32_t      avg_time;
} modbus_master_driver_t;

static void mb_master_driver_thread_init(evloop_thread_t* thrd);
static void mb_master_driver_thread_fini(evloop_thread_t* thrd);

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
static LIST_HEAD_DECL(_modbus_masters);

static evloop_thread_t    _mb_master_driver_thread =
{
  .init = mb_master_driver_thread_init,
  .fini = mb_master_driver_thread_fini,
};

////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
////////////////////////////////////////////////////////////////////////////////
static modbus_register_t*
__get_modbus_register(modbus_master_driver_t* master, uint32_t slave_id, modbus_reg_type_t reg_type, uint32_t addr)
{
  return modbus_register_list_lookup_by_mb_type_addr(&master->reg_map,
                                                     slave_id,
                                                     reg_type,
                                                     addr);
}

static bool
__encode_modbus_register(modbus_master_driver_t* master, uint8_t slave_addr, modbus_reg_type_t reg_type, uint32_t reg_addr, uint16_t* ret)
{
  modbus_register_t*    mreg;
  uint16_t              v = 0;

  mreg = __get_modbus_register(master, slave_addr, reg_type, reg_addr);
  if(mreg == NULL)
  {
    TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", modbus_reg_coil, slave_addr, reg_addr);
    return FALSE;
  }

  if(mreg->codec.d_mask != 0)
  {
    v = (uint32_t)channel_manager_get_raw_value(mreg->chnl_num);
    v = ((v << mreg->codec.d_shift) & mreg->codec.d_mask);
  }
  else
  {
    v = 0;
  }

  if(mreg->codec.s_mask != 0)
  {
    if(channel_manager_get_sensor_fault_status(mreg->chnl_num))
    {
      v |= ((mreg->codec.fault << mreg->codec.s_shift) & mreg->codec.s_mask);
    }
    else
    {
      v |= ((mreg->codec.ok << mreg->codec.s_shift) & mreg->codec.s_mask);
    }
  }

  *ret = v;
  return TRUE;
}

static bool
__decode_modbus_register(modbus_master_driver_t* master, uint8_t slave_addr, modbus_reg_type_t reg_type, uint32_t reg_addr, uint16_t v)
{
  modbus_register_t*    mreg;
  uint16_t              final_v;

  mreg = __get_modbus_register(master, slave_addr, reg_type, reg_addr);
  if(mreg == NULL)
  {
    TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", reg_type, slave_addr, reg_addr);
    return FALSE;
  }

  if(mreg->codec.d_mask != 0)
  {
    // data response
    final_v = u16_filter(mreg->codec.d_mask, mreg->codec.d_shift, v);
    channel_manager_set_raw_value(mreg->chnl_num, final_v);
  }

  if(mreg->codec.s_mask != 0)
  {
    // sensor status response
    final_v = u16_filter(mreg->codec.s_mask, mreg->codec.s_shift, v);
    channel_manager_set_sensor_fault_status(mreg->chnl_num, final_v == mreg->codec.fault ? TRUE : FALSE);
  }
  return TRUE;
}

static inline void
mark_req_start_time(modbus_master_driver_t* master)
{
  master->req_start_time = time_util_get_sys_clock_in_ms();
}

static inline unsigned long
get_time_took_for_transaction_in_ms(modbus_master_driver_t* master)
{
  unsigned long now;

  now = time_util_get_sys_clock_in_ms();

  return now - master->req_start_time;
}

////////////////////////////////////////////////////////////////////////////////
//
// modbus master request handler
//
////////////////////////////////////////////////////////////////////////////////
static void
modbus_master_driver_coil_write_req(modbus_master_driver_t* master, modbus_master_driver_request_config_t* req)
{
  ModbusMasterCTX* ctx = master->ctx;
  uint8_t           slave_addr;
  uint16_t          reg_addr;
  uint16_t          num_regs;
  uint16_t          v;
  uint8_t           bounce_buffer[APP_MODBUS_MAXTER_BOUNCE_BUFFER_SIZE];

  slave_addr  = (uint8_t)req->slave_id;
  reg_addr    = (uint16_t)req->start_addr;
  num_regs    = (uint16_t)req->num_regs;

  if(num_regs == 1)
  {
    if(__encode_modbus_register(master, slave_addr, modbus_reg_coil, reg_addr, &v) == FALSE)
    {
      TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", modbus_reg_coil, slave_addr, reg_addr);
      return;
    }
    mb_master_write_single_coil(ctx, slave_addr, reg_addr, v == 0 ? 0 : 1);
    return;
  }

  memset(bounce_buffer, 0, num_regs / 8 + 1);
  for(uint16_t i = 0; i < num_regs; i++)
  {
    if(__encode_modbus_register(master, slave_addr, modbus_reg_coil, reg_addr + i, &v) == FALSE)
    {
      TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", modbus_reg_coil, slave_addr, reg_addr + i);
      return;
    }

    xMBUtilSetBits(bounce_buffer, i, 1, v == 0 ? 0 : 1);
  }

  mb_master_write_multiple_coils(ctx, slave_addr, reg_addr, num_regs, bounce_buffer);
}

static void
modbus_master_driver_holding_write_req(modbus_master_driver_t* master, modbus_master_driver_request_config_t* req)
{
  ModbusMasterCTX* ctx = master->ctx;
  uint8_t           slave_addr;
  uint16_t          reg_addr;
  uint16_t          num_regs;
  uint16_t          v;
  uint16_t          bounce_buffer[APP_MODBUS_MAXTER_BOUNCE_BUFFER_SIZE];
  uint8_t*          u8Ptr;

  slave_addr  = (uint8_t)req->slave_id;
  reg_addr    = (uint16_t)req->start_addr;
  num_regs    = (uint16_t)req->num_regs;

  if(num_regs == 1)
  {
    if(__encode_modbus_register(master, slave_addr, modbus_reg_holding, reg_addr, &v) == FALSE)
    {
      TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", modbus_reg_holding, slave_addr, reg_addr);
      return;
    }

    mb_master_write_single_register(ctx, slave_addr, reg_addr, (uint16_t)v);
    return;
  }

  for(uint16_t i = 0; i < num_regs; i++)
  {
    if(__encode_modbus_register(master, slave_addr, modbus_reg_holding, reg_addr + i, &v) == FALSE)
    {
      TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", modbus_reg_holding, slave_addr, reg_addr + i);
      return;
    }

    u8Ptr = (uint8_t*)&bounce_buffer[i];
    U16_TO_BUFFER((uint16_t)v, u8Ptr);
  }

  mb_master_write_multiple_registers(ctx, slave_addr, reg_addr, num_regs, bounce_buffer);
}

static void
modbus_master_driver_request(modbus_master_driver_t* master)
{
  modbus_master_driver_request_config_t*   req;
  ModbusMasterCTX* ctx = master->ctx;
  uint8_t           slave_addr;
  uint16_t          reg_addr;
  uint16_t          num_regs;

  master->n_request++;

  req = &master->request_schedule[master->current_request];

  slave_addr  = (uint8_t)req->slave_id;
  reg_addr    = (uint16_t)req->start_addr;
  num_regs    = (uint16_t)req->num_regs;

  switch(req->reg)
  {
  case modbus_reg_coil:
    if(req->op == modbus_reg_op_read)
    {
      mb_master_read_coils_req(ctx, slave_addr, reg_addr, num_regs);
    }
    else
    {
      modbus_master_driver_coil_write_req(master, req);
    }
    break;

  case modbus_reg_discrete:
    mb_master_read_discrete_inputs(ctx, slave_addr, reg_addr, num_regs);
    break;

  case modbus_reg_holding:
    if(req->op == modbus_reg_op_read)
    {
      mb_master_read_holding_registers(ctx, slave_addr, reg_addr, num_regs);
    }
    else
    {
      modbus_master_driver_holding_write_req(master, req);
    }
    break;

  case modbus_reg_input:
    mb_master_read_input_registers(ctx, slave_addr, reg_addr, num_regs);
    break;
  }

  mark_req_start_time(master);
  evloop_timer_start(&master->transaction_timer, master->timeout / 1000.0, 0);
}

static void
modbus_driver_master_next(modbus_master_driver_t* master)
{
  double wait_time;
  double target_delay;
  unsigned long   time_took_for_prev_transacion = get_time_took_for_transaction_in_ms(master);

  //
  // not entire accurate but good enough
  //
  if(master->min_transaction_time == 0 ||
     master->min_transaction_time > time_took_for_prev_transacion)
  {
    master->min_transaction_time = time_took_for_prev_transacion;
  }

  if(master->max_transaction_time == 0 ||
     master->max_transaction_time < time_took_for_prev_transacion)
  {
    master->max_transaction_time = time_took_for_prev_transacion;
  }

  // not entire accurate but close enough
  master->avg_time = (master->avg_time + time_took_for_prev_transacion)/2;

  master->current_request++;

  if(master->current_request >= master->num_reqs)
  {
    // schedule finish delay
    master->current_request = 0;
    if(master->schedule_finish_delay == 0)
    {
      // use inter request delay instead
      target_delay = master->inter_request_delay;
    }
    else
    {
      target_delay = master->schedule_finish_delay;
    }
  }
  else
  {
    // inter request delay
    target_delay = master->inter_request_delay;
  }

  wait_time = target_delay - time_took_for_prev_transacion;

#if 0
  TRACE(MBM_DRIVER, "took %d, target: %f, wait %f\n",
      time_took_for_prev_transacion,
      target_delay,
      wait_time);
#endif

  if(wait_time >= 1)
  {
    evloop_timer_start(&master->wait_timer, wait_time / 1000.0, 0);
  }
  else
  {
    modbus_master_driver_request(master);
  }
}

static void
modbus_master_wait_timeout(evloop_timer_t* te, void* unused)
{
  modbus_master_driver_t* master = container_of(te, modbus_master_driver_t, wait_timer);

  modbus_master_driver_request(master);
}

static void
modbus_master_transaction_timeout(evloop_timer_t* te, void* unused)
{
  modbus_master_driver_t* master = container_of(te, modbus_master_driver_t, transaction_timer);
  modbus_master_driver_request_config_t*   req;

  master->n_rsp_timeout++;

  req = &master->request_schedule[master->current_request];

  TRACE(MBM_DRIVER, "tr timeout, req ndx %d: %d, %d, %d, %d, %d\n",
      master->current_request,
      req->slave_id,
      req->op,
      req->reg,
      req->start_addr,
      req->num_regs);

  modbus_driver_master_next(master);
}

////////////////////////////////////////////////////////////////////////////////
//
// modbus master event callbacks
//
////////////////////////////////////////////////////////////////////////////////
static void
__handle_coil_discrete_read_response(modbus_master_driver_t* master, modbus_reg_type_t reg_type,
    uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* data_buf)
{
  uint16_t      i,
                current;
  uint16_t      v;

  for(i = 0; i < nreg; i++)
  {
    current = addr + i;

    v = xMBUtilGetBits(data_buf, i, 1);
    if(__decode_modbus_register(master, slave, reg_type, current, v) == FALSE)
    {
      TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", reg_type, slave, current);
      continue;
    }
  }
}

static void
__handle_holding_input_read_response(modbus_master_driver_t* master, modbus_reg_type_t reg_type,
    uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* data_buf)
{
  uint16_t      i,
                current;
  uint16_t      v;

  for(i = 0; i < nreg; i++, data_buf += 2)
  {
    current = addr + i;

    v = BUFFER_TO_U16(data_buf);
    if(__decode_modbus_register(master, slave, reg_type, current, v) == FALSE)
    {
      TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", reg_type, slave, current);
      continue;
    }
  }
}

static void
__input_regs_cb(ModbusMasterCTX* ctx, uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* regs)
{
  modbus_master_driver_t* master = (modbus_master_driver_t*)ctx->priv;

  master->n_response++;

  evloop_timer_stop(&master->transaction_timer);

  __handle_holding_input_read_response(master, modbus_reg_input, slave, addr, nreg, regs);

  modbus_driver_master_next(master);
}

static void
__holding_regs_cb(ModbusMasterCTX* ctx, uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* regs, MBRegisterMode mode)
{
  modbus_master_driver_t* master = (modbus_master_driver_t*)ctx->priv;

  master->n_response++;

  evloop_timer_stop(&master->transaction_timer);

  if(mode == MB_REG_READ)
  {
    __handle_holding_input_read_response(master, modbus_reg_holding, slave, addr, nreg, regs);
  }

  modbus_driver_master_next(master);
}

static void
__coil_cb(ModbusMasterCTX* ctx, uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* regs, MBRegisterMode mode)
{
  modbus_master_driver_t* master = (modbus_master_driver_t*)ctx->priv;

  master->n_response++;

  evloop_timer_stop(&master->transaction_timer);

  if(mode == MB_REG_READ)
  {
    __handle_coil_discrete_read_response(master,  modbus_reg_coil, slave, addr, nreg, regs);
  }

  modbus_driver_master_next(master);
}

static void
__discrete_cb(ModbusMasterCTX* ctx, uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* regs)
{
  modbus_master_driver_t* master = (modbus_master_driver_t*)ctx->priv;

  master->n_response++;

  evloop_timer_stop(&master->transaction_timer);

  __handle_coil_discrete_read_response(master,  modbus_reg_discrete, slave, addr, nreg, regs);

  modbus_driver_master_next(master);
}

static void
__modbus_master_event_cb(ModbusMasterCTX* ctx, modbus_master_event_t event)
{
  modbus_master_driver_t* master;

  master = (modbus_master_driver_t*)ctx->priv;

  switch(event)
  {
  case modbus_master_event_connected:
    TRACE(MBM_DRIVER, "xxxxxx connected callback xxxxxx\n");
    master->current_request = 0;
    modbus_master_driver_request(master);
    break;

  case modbus_master_event_disconnected:
    TRACE(MBM_DRIVER, "xxxxxx disconnected callback xxxxxx\n");
    if(evloop_timer_active(&master->transaction_timer))
    {
      evloop_timer_stop(&master->transaction_timer);
    }

    if(evloop_timer_active(&master->wait_timer))
    {
      evloop_timer_stop(&master->wait_timer);
    }

    master->current_request = 0;

    // FIXME sensor fault handling
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
////////////////////////////////////////////////////////////////////////////////
static modbus_master_driver_t*
alloc_init_modbus_master(modbus_master_driver_config_t* cfg)
{
  modbus_master_driver_t*     master;
  ModbusMasterCTX*            ctx;

  master = malloc_zero(sizeof(modbus_master_driver_t));
  if(master == NULL)
  {
    TRACE(MBM_DRIVER,"failed to alloc modbus_master_driver_t\n");
    goto failed;
  }

  INIT_LIST_HEAD(&master->le);

  master->mb_type   = cfg->protocol;

  if(master->mb_type == modbus_master_driver_type_tcp)
  {
    ModbusTCPMaster*    tcp_master;

    tcp_master = malloc_zero(sizeof(ModbusTCPMaster));
    if(tcp_master == NULL)
    {
      TRACE(MBM_DRIVER, "failed to alloc ModbusTCPMaster\n");
      goto failed;
    }

    ctx = &tcp_master->ctx;
    modbus_tcp_master_init_with_ip_port(tcp_master, cfg->dest_ip, cfg->dest_port);
  }
  else
  {
    ModbusRTUMaster*    rtu_master;

    rtu_master = malloc_zero(sizeof(ModbusRTUMaster));
    if(rtu_master == NULL)
    {
      TRACE(MBM_DRIVER, "failed to alloc ModbusRTUMaster\n");
      goto failed;
    }

    ctx = &rtu_master->ctx;
    modbus_rtu_master_init(rtu_master, cfg->serial_port, &cfg->serial_cfg);
  }

  ctx->input_regs_cb      = __input_regs_cb;
  ctx->holding_regs_cb    = __holding_regs_cb;
  ctx->coil_cb            = __coil_cb;
  ctx->discrete_cb        = __discrete_cb;
  ctx->event_cb           = __modbus_master_event_cb;

  master->schedule_finish_delay = cfg->schedule_finish_delay;
  master->inter_request_delay   = cfg->inter_request_delay;
  master->timeout               = cfg->timeout;

  master->ctx = ctx;
  ctx->priv   = (void*)master;

  modbus_register_list_init(&master->reg_map);

  evloop_timer_init(&master->wait_timer, modbus_master_wait_timeout, NULL);
  evloop_timer_init(&master->transaction_timer, modbus_master_transaction_timeout, NULL);

  list_add_tail(&master->le, &_modbus_masters);

  master->n_request         = 0;
  master->n_response        = 0;
  master->n_rsp_timeout     = 0;

  master->min_transaction_time  = 0;
  master->max_transaction_time  = 0;

  return master;

failed:
  exit(-1);
  return NULL;
}

static void
modbus_driver_load_masters(void)
{
  modbus_master_driver_config_t   cfg;
  int                             num_masters,
                                  num_regs,
                                  num_reqs,
                                  i;
  modbus_master_driver_t*         master;

  num_masters = cfg_mgr_get_num_modbus_masters();

  for(i = 0; i < num_masters; i++)
  {
    cfg_mgr_get_modbus_master_at(i, &cfg);

    if(cfg.protocol == modbus_master_driver_type_tcp)
    {
      TRACE(MBM_DRIVER,"initializing modbus tcp master for %s:%d\n", cfg.dest_ip, cfg.dest_port);
    }
    else
    {
      TRACE(MBM_DRIVER, "initializing modbus rtu master, port %s\n", cfg.serial_port);
    }

    master = alloc_init_modbus_master(&cfg);

    // load registers 
    num_regs = cfg_mgr_get_modbus_master_num_regs(i);
    for(int reg_ndx = 0; reg_ndx < num_regs; reg_ndx++)
    {
      uint32_t                    chnl;
      modbus_address_t            reg;
      modbus_reg_codec_t          codec;

      cfg_mgr_get_modbus_master_reg(i, reg_ndx, &reg, &chnl, &codec);
      modbus_register_list_add(&master->reg_map,
          reg.slave_id, reg.reg_type, reg.mb_address,
          chnl, modbus_reg_mapping_to_channel,
          &codec);
    }

    // load request schedule
    num_reqs = cfg_mgr_get_modbus_master_num_request_schedules(i);
    master->num_reqs = num_reqs;
    master->request_schedule  = malloc_zero(sizeof(modbus_master_driver_request_config_t) * num_reqs);
    if(master->request_schedule == NULL)
    {
      TRACE(MBM_DRIVER,"failed to malloc request_schedule\n");
      exit(-1);
    }

    master->current_request   = 0;

    for(int req_ndx = 0; req_ndx < num_reqs; req_ndx++)
    {
      cfg_mgr_get_modbus_slave_request_schedule(i, req_ndx, &master->request_schedule[req_ndx]);
    }
  }

  // start phase
  list_for_each_entry(master, &_modbus_masters, le)
  {
    if(master->mb_type == modbus_master_driver_type_tcp)
    {
      modbus_tcp_master_start((ModbusTCPMaster*)master->ctx);
    }
    else
    {
      modbus_rtu_master_start((ModbusRTUMaster*)master->ctx);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// thread initializer
//
////////////////////////////////////////////////////////////////////////////////
static void
mb_master_driver_thread_init(evloop_thread_t* thrd)
{
  TRACE(MBM_DRIVER, "loading masters from config\n");

  modbus_driver_load_masters();

  TRACE(MBM_DRIVER, "done loading masters\n");

  app_init_complete_signal();
}

static void
mb_master_driver_thread_fini(evloop_thread_t* thrd)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
modbus_master_driver_init(void)
{
  TRACE(MBM_DRIVER, "starting modbus master driver\n");

  evloop_thread_init(&_mb_master_driver_thread);
  evloop_thread_run(&_mb_master_driver_thread);

  app_init_complete_wait();
}

////////////////////////////////////////////////////////////////////////////////
//
// external API implementation
//
////////////////////////////////////////////////////////////////////////////////
/**
 * return object
 *
 {
  "master_list": [
    "master_type":      "tcp" or "rtu",

    "in case of tcp":
    "server_ip":    "xxx.xxx.xxx.xxx",
    "server_port":  xxx,
    "connected":    true or false,

    "in case of rtu":
    ...

    FIXME
    master stats
  ]
 }
 */
cJSON*
modbus_master_driver_get_stat(void)
{
  cJSON*                    ret;
  cJSON*                    jmaster_list;
  cJSON*                    master_stat;
  modbus_master_driver_t*   master;

  evloop_thread_lock(&_mb_master_driver_thread);

  ret = cJSON_CreateObject();
  jmaster_list = cJSON_CreateArray();
  list_for_each_entry(master, &_modbus_masters, le)
  {
    if(master->mb_type == modbus_master_driver_type_tcp)
    {
      ModbusTCPMaster*    tcp_master;

      tcp_master = container_of(master->ctx, ModbusTCPMaster, ctx);
      master_stat = modbus_tcp_master_get_stat(tcp_master);
    }
    else
    {
      ModbusRTUMaster*    rtu_master;

      rtu_master = container_of(master->ctx, ModbusRTUMaster, ctx);
      master_stat = modbus_rtu_master_get_stat(rtu_master);
    }

    cJSON_AddNumberToObject(master_stat, "n_request", master->n_request);
    cJSON_AddNumberToObject(master_stat, "n_response", master->n_response);
    cJSON_AddNumberToObject(master_stat, "n_rsp_timeout", master->n_rsp_timeout);
    cJSON_AddNumberToObject(master_stat, "min_transaction_time", master->min_transaction_time);
    cJSON_AddNumberToObject(master_stat, "max_transaction_time", master->max_transaction_time);
    cJSON_AddNumberToObject(master_stat, "avg_time", master->avg_time);
    
    cJSON_AddItemToArray(jmaster_list, master_stat);
  }

  cJSON_AddItemToObject(ret, "master_list", jmaster_list);

  evloop_thread_unlock(&_mb_master_driver_thread);

  return ret;
}

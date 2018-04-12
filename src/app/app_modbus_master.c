#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "common_def.h"
#include "app_modbus_master.h"
#include "app_config.h"
#include "list.h"
#include "modbus_master.h"
#include "modbus_tcp_master.h"
#include "modbus_rtu_master.h"
#include "modbus_util.h"
#include "channel_manager.h"
#include "evloop_timer.h"
#include "trace.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
#define APP_MODBUS_MAXTER_BOUNCE_BUFFER_SIZE          256

typedef struct
{
  struct list_head          le;
  app_modbus_master_type_t  mb_type;
  ModbusMasterCTX*          ctx;
  modbus_register_list_t    reg_map;
  evloop_timer_t            transaction_timer;
  int                       current_request;
  int                       num_reqs;
  app_modbus_master_request_config_t*   request_schedule;
} app_modbus_master_t;

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
static LIST_HEAD(_modbus_masters);

////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
////////////////////////////////////////////////////////////////////////////////
static int32_t
__get_mapped_channel(app_modbus_master_t* master, uint32_t slave_id, modbus_reg_type_t reg_type, uint32_t addr)
{
  modbus_register_t*  reg;

  reg = modbus_register_list_lookup_by_mb_type_addr(&master->reg_map,
                                                    slave_id,
                                                    reg_type,
                                                    addr);

  if(reg == NULL)
  {
    return -1;
  }
  return reg->chnl_num;
}

////////////////////////////////////////////////////////////////////////////////
//
// modbus master request handler
//
////////////////////////////////////////////////////////////////////////////////
static void
app_modbus_master_coil_write_req(app_modbus_master_t* master, app_modbus_master_request_config_t* req)
{
  ModbusMasterCTX* ctx = master->ctx;
  uint8_t           slave_addr;
  uint16_t          reg_addr;
  uint16_t          num_regs;
  int32_t           chnl_num;
  uint32_t          v;
  uint8_t           bounce_buffer[APP_MODBUS_MAXTER_BOUNCE_BUFFER_SIZE];

  slave_addr  = (uint8_t)req->slave_id;
  reg_addr    = (uint16_t)req->start_addr;
  num_regs    = (uint16_t)req->num_regs;

  if(num_regs == 1)
  {
    chnl_num = __get_mapped_channel(master, slave_addr, modbus_reg_coil, reg_addr);
    if(chnl_num == -1)
    {
      TRACE(APP_START, "can't find channel for %d, %d:%d\n", modbus_reg_coil, slave_addr, reg_addr);
      return;
    }

    v = (uint32_t)channel_manager_get_raw_value((uint32_t)chnl_num);
    mb_master_write_single_coil(ctx, slave_addr, reg_addr, v == 0 ? 0 : 1);
    return;
  }

  memset(bounce_buffer, 0, num_regs / 8 + 1);
  for(uint16_t i = 0; i < num_regs; i++)
  {
    chnl_num = __get_mapped_channel(master, slave_addr, modbus_reg_coil, reg_addr + i);
    if(chnl_num == -1)
    {
      TRACE(APP_START, "can't find channel for %d, %d:%d\n", modbus_reg_coil, slave_addr, reg_addr + i);
      return;
    }

    v = (uint32_t)channel_manager_get_raw_value((uint32_t)chnl_num);

    xMBUtilSetBits(bounce_buffer, i, 1, v == 0 ? 0 : 1);
  }

  mb_master_write_multiple_coils(ctx, slave_addr, reg_addr, num_regs, bounce_buffer);
}

static void
app_modbus_master_holding_write_req(app_modbus_master_t* master, app_modbus_master_request_config_t* req)
{
  ModbusMasterCTX* ctx = master->ctx;
  uint8_t           slave_addr;
  uint16_t          reg_addr;
  uint16_t          num_regs;
  int32_t           chnl_num;
  uint32_t          v;
  uint16_t          bounce_buffer[APP_MODBUS_MAXTER_BOUNCE_BUFFER_SIZE];
  uint8_t*          u8Ptr;

  slave_addr  = (uint8_t)req->slave_id;
  reg_addr    = (uint16_t)req->start_addr;
  num_regs    = (uint16_t)req->num_regs;

  if(num_regs == 1)
  {
    chnl_num = __get_mapped_channel(master, slave_addr, modbus_reg_holding, reg_addr);
    if(chnl_num == -1)
    {
      TRACE(APP_START, "can't find channel for %d, %d:%d\n", modbus_reg_holding, slave_addr, reg_addr);
      return;
    }

    v = (uint32_t)channel_manager_get_raw_value((uint32_t)chnl_num);
    mb_master_write_single_register(ctx, slave_addr, reg_addr, (uint16_t)v);
    return;
  }

  for(uint16_t i = 0; i < num_regs; i++)
  {
    chnl_num = __get_mapped_channel(master, slave_addr, modbus_reg_holding, reg_addr + i);
    if(chnl_num == -1)
    {
      TRACE(APP_START, "can't find channel for %d, %d:%d\n", modbus_reg_holding, slave_addr, reg_addr + i);
      return;
    }

    v = (uint32_t)channel_manager_get_raw_value((uint32_t)chnl_num);

    u8Ptr = (uint8_t*)&bounce_buffer[i];
    U16_TO_BUFFER((uint16_t)v, u8Ptr);
  }

  mb_master_write_multiple_registers(ctx, slave_addr, reg_addr, num_regs, bounce_buffer);
}

static void
app_modbus_master_request(app_modbus_master_t* master)
{
  app_modbus_master_request_config_t*   req;
  ModbusMasterCTX* ctx = master->ctx;
  uint8_t           slave_addr;
  uint16_t          reg_addr;
  uint16_t          num_regs;

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
      app_modbus_master_coil_write_req(master, req);
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
      app_modbus_master_holding_write_req(master, req);
    }
    break;

  case modbus_reg_input:
    mb_master_read_input_registers(ctx, slave_addr, reg_addr, num_regs);
    break;
  }

  evloop_timer_start(&master->transaction_timer, 2.0, 0);
}

static void
app_modbus_master_next(app_modbus_master_t* master)
{
  master->current_request = (master->current_request + 1) % master->num_reqs;
  app_modbus_master_request(master);
}

static void
modbus_master_transaction_timeout(evloop_timer_t* te, void* unused)
{
  app_modbus_master_t* master = container_of(te, app_modbus_master_t, transaction_timer);
  app_modbus_master_request_config_t*   req;

  req = &master->request_schedule[master->current_request];

  TRACE(APP_START, "tr timeout, req ndx %d: %d, %d, %d, %d, %d\n",
      master->current_request,
      req->slave_id,
      req->op,
      req->reg,
      req->start_addr,
      req->num_regs);

  app_modbus_master_next(master);
}

////////////////////////////////////////////////////////////////////////////////
//
// modbus master event callbacks
//
////////////////////////////////////////////////////////////////////////////////
static void
__handle_coil_discrete_read_response(app_modbus_master_t* master, modbus_reg_type_t reg_type,
    uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* data_buf)
{
  uint16_t      i,
                current;
  int32_t       chnl_num;
  uint8_t       v;

  for(i = 0; i < nreg; i++)
  {
    current = addr + i;

    chnl_num = __get_mapped_channel(master, slave, reg_type, current);
    if(chnl_num == -1)
    {
      TRACE(APP_START, "can't find channel for %d, %d:%d\n", reg_type, slave, current);
      continue;
    }

    v = xMBUtilGetBits(data_buf, i, 1);

    channel_manager_set_raw_value(chnl_num, v == 0 ? 0 : 1);
  }
}

static void
__handle_holding_input_read_response(app_modbus_master_t* master, modbus_reg_type_t reg_type,
    uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* data_buf)
{
  uint16_t      i,
                current;
  int32_t       chnl_num;
  uint16_t      v;

  for(i = 0; i < nreg; i++, data_buf += 2)
  {
    current = addr + i;

    chnl_num = __get_mapped_channel(master, slave, reg_type, current);
    if(chnl_num == -1)
    {
      TRACE(APP_START, "can't find channel for %d, %d:%d\n", reg_type, slave, current);
      continue;
    }

    v = BUFFER_TO_U16(data_buf);

    channel_manager_set_raw_value(chnl_num, v);
  }
}

static void
__input_regs_cb(ModbusMasterCTX* ctx, uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* regs)
{
  app_modbus_master_t* master = (app_modbus_master_t*)ctx->priv;

  evloop_timer_stop(&master->transaction_timer);

  __handle_holding_input_read_response(master, modbus_reg_input, slave, addr, nreg, regs);

  app_modbus_master_next(master);
}

static void
__holding_regs_cb(ModbusMasterCTX* ctx, uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* regs, MBRegisterMode mode)
{
  app_modbus_master_t* master = (app_modbus_master_t*)ctx->priv;

  evloop_timer_stop(&master->transaction_timer);

  if(mode == MB_REG_READ)
  {
    __handle_holding_input_read_response(master, modbus_reg_holding, slave, addr, nreg, regs);
  }

  app_modbus_master_next(master);
}

static void
__coil_cb(ModbusMasterCTX* ctx, uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* regs, MBRegisterMode mode)
{
  app_modbus_master_t* master = (app_modbus_master_t*)ctx->priv;

  evloop_timer_stop(&master->transaction_timer);

  if(mode == MB_REG_READ)
  {
    __handle_coil_discrete_read_response(master,  modbus_reg_coil, slave, addr, nreg, regs);
  }

  app_modbus_master_next(master);
}

static void
__discrete_cb(ModbusMasterCTX* ctx, uint8_t slave, uint16_t addr, uint16_t nreg, uint8_t* regs)
{
  app_modbus_master_t* master = (app_modbus_master_t*)ctx->priv;

  evloop_timer_stop(&master->transaction_timer);

  __handle_coil_discrete_read_response(master,  modbus_reg_discrete, slave, addr, nreg, regs);

  app_modbus_master_next(master);
}

static void
__modbus_master_event_cb(ModbusMasterCTX* ctx, modbus_master_event_t event)
{
  app_modbus_master_t* master;

  master = (app_modbus_master_t*)ctx->priv;

  switch(event)
  {
  case modbus_master_event_connected:
    TRACE(APP_START, "xxxxxx connected callback xxxxxx\n");
    master->current_request = 0;
    app_modbus_master_request(master);

    break;

  case modbus_master_event_disconnected:
    TRACE(APP_START, "xxxxxx disconnected callback xxxxxx\n");
    if(evloop_timer_active(&master->transaction_timer))
    {
      evloop_timer_stop(&master->transaction_timer);
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
static app_modbus_master_t*
alloc_init_modbus_master(app_modbus_master_config_t* cfg)
{
  app_modbus_master_t*    master;
  ModbusMasterCTX*        ctx;

  master = malloc(sizeof(app_modbus_master_t));
  if(master == NULL)
  {
    TRACE(APP_START,"failed to alloc app_modbus_master_t\n");
    goto failed;
  }

  INIT_LIST_HEAD(&master->le);

  master->mb_type   = cfg->protocol;

  if(master->mb_type == app_modbus_master_type_tcp)
  {
    ModbusTCPMaster*    tcp_master;

    tcp_master = malloc(sizeof(ModbusTCPMaster));
    if(tcp_master == NULL)
    {
      TRACE(APP_START, "failed to alloc ModbusTCPMaster\n");
      goto failed;
    }

    ctx = &tcp_master->ctx;
    modbus_tcp_master_init_with_ip_port(tcp_master, cfg->dest_ip, cfg->dest_port);
  }
  else
  {
    ModbusRTUMaster*    rtu_master;

    rtu_master = malloc(sizeof(ModbusRTUMaster));
    if(rtu_master == NULL)
    {
      TRACE(APP_START, "failed to alloc ModbusRTUMaster\n");
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

  master->ctx = ctx;
  ctx->priv   = (void*)master;

  modbus_register_list_init(&master->reg_map);

  evloop_timer_init(&master->transaction_timer, modbus_master_transaction_timeout, NULL);

  list_add_tail(&master->le, &_modbus_masters);

  return master;

failed:
  exit(-1);
  return NULL;
}

static void
app_modbus_load_masters(void)
{
  app_modbus_master_config_t      cfg;
  int                             num_masters,
                                  num_regs,
                                  num_reqs,
                                  i;
  app_modbus_master_t*            master;

  num_masters = app_config_get_num_modbus_masters();

  for(i = 0; i < num_masters; i++)
  {
    app_config_get_modbus_master_at(i, &cfg);

    if(cfg.protocol == app_modbus_master_type_tcp)
    {
      TRACE(APP_START,"initializing modbus tcp master for %s:%d\n", cfg.dest_ip, cfg.dest_port);
    }
    else
    {
      TRACE(APP_START, "initializing modbus rtu master, port %s\n", cfg.serial_port);
    }

    master = alloc_init_modbus_master(&cfg);

    // load registers 
    num_regs = app_config_get_modbus_master_num_regs(i);
    for(int reg_ndx = 0; reg_ndx < num_regs; reg_ndx++)
    {
      uint32_t          chnl;
      modbus_address_t  reg;

      app_config_get_modbus_master_reg(i, reg_ndx, &reg, &chnl);
      modbus_register_list_add(&master->reg_map,
          reg.slave_id, reg.reg_type, reg.mb_address, chnl);
    }

    // load request schedule
    num_reqs = app_config_get_modbus_master_num_request_schedules(i);
    master->num_reqs = num_reqs;
    master->request_schedule  = malloc(sizeof(app_modbus_master_request_config_t) * num_reqs);
    if(master->request_schedule == NULL)
    {
      TRACE(APP_START,"failed to malloc request_schedule\n");
      exit(-1);
    }

    master->current_request   = 0;

    for(int req_ndx = 0; req_ndx < num_reqs; req_ndx++)
    {
      app_config_get_modbus_slave_request_schedule(i, req_ndx, &master->request_schedule[req_ndx]);
    }
  }

  // start phase
  list_for_each_entry(master, &_modbus_masters, le)
  {
    if(master->mb_type == app_modbus_master_type_tcp)
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
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
app_modbus_master_init(void)
{
  app_modbus_load_masters();
}

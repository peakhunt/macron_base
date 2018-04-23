#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "common_def.h"
#include "modbus_slave_driver.h"
#include "cfg_mgr.h"
#include "list.h"
#include "trace.h"
#include "channel_manager.h"
#include "modbus_rtu_slave.h"
#include "modbus_tcp_slave.h"
#include "modbus_util.h"
#include "modbus_regs.h"
#include "app_init_completion.h"
#include "bit_util.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  struct list_head            le;
  modbus_slave_driver_type_t  mb_type;
  ModbusSlaveCTX              *ctx;
  modbus_register_list_t      reg_map;
} modbus_slave_driver_t;

static void mb_slave_driver_thread_init(evloop_thread_t* thrd);
static void mb_slave_driver_thread_fini(evloop_thread_t* thrd);

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
static LIST_HEAD_DECL(_modbus_slaves);
static int _num_modbus_slaves = 0;

static evloop_thread_t    _mb_slave_driver_thread =
{
  .init = mb_slave_driver_thread_init,
  .fini = mb_slave_driver_thread_fini,
};

////////////////////////////////////////////////////////////////////////////////
//
// common modbus request handlers
//
////////////////////////////////////////////////////////////////////////////////
static modbus_register_t*
__get_modbus_register(modbus_slave_driver_t* slave, uint32_t slave_id, modbus_reg_type_t reg_type, uint32_t addr)
{
  return modbus_register_list_lookup_by_mb_type_addr(&slave->reg_map,
                                                     slave_id,
                                                     reg_type,
                                                     addr);
}

static bool
__encode_modbus_register(modbus_slave_driver_t* slave, uint8_t slave_id, modbus_reg_type_t reg_type, uint32_t reg_addr, uint16_t* ret)
{
  modbus_register_t*    mreg;
  uint16_t              v = 0;

  mreg = __get_modbus_register(slave, slave_id, reg_type, reg_addr);
  if(mreg == NULL)
  {
    TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", modbus_reg_coil, slave_id, reg_addr);
    return FALSE;
  }

  if(mreg->filter.d_mask != 0)
  {
    v = (uint32_t)channel_manager_get_raw_value(mreg->chnl_num);
    v = ((v & mreg->filter.d_mask) << mreg->filter.d_shift);
  }
  else
  {
    v = 0;
  }

  if(mreg->filter.s_mask != 0)
  {
    if(channel_manager_get_sensor_fault_status(mreg->chnl_num))
    {
      v |= ((mreg->filter.fault << mreg->filter.s_shift) & mreg->filter.s_mask);
    }
  }

  *ret = v;
  return TRUE;
}

static bool
__decode_modbus_register(modbus_slave_driver_t* slave, uint8_t slave_id, modbus_reg_type_t reg_type, uint32_t reg_addr, uint16_t v)
{
  modbus_register_t*    mreg;
  uint16_t              final_v;

  mreg = __get_modbus_register(slave, slave_id, reg_type, reg_addr);
  if(mreg == NULL)
  {
    TRACE(MBM_DRIVER, "can't find channel for %d, %d:%d\n", reg_type, slave_id, reg_addr);
    return FALSE;
  }

  if(mreg->filter.d_mask != 0)
  {
    // data response
    final_v = u16_filter(mreg->filter.d_mask, mreg->filter.d_shift, v);
    channel_manager_set_raw_value(mreg->chnl_num, final_v);
  }

  if(mreg->filter.s_mask != 0)
  {
    // sensor status response
    final_v = u16_filter(mreg->filter.s_mask, mreg->filter.s_shift, v);
    channel_manager_set_sensor_fault_status(mreg->chnl_num, final_v == mreg->filter.fault ? TRUE : FALSE);
  }
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
// common modbus request handlers
//
////////////////////////////////////////////////////////////////////////////////
static MBErrorCode
__handle_modbus_u16(ModbusSlaveCTX* ctx, modbus_reg_type_t reg_type, uint8_t* bufPtr,
    uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  uint16_t                current_addr,
                          end_addr;
  uint8_t*                buffer;
  uint16_t                v;
  modbus_slave_driver_t   *slave;

  current_addr  = usAddress;
  end_addr      = current_addr + usNRegs - 1;
  buffer        = bufPtr;

  slave = (modbus_slave_driver_t*)ctx->priv;

  for(; current_addr <= end_addr; current_addr++, buffer += 2)
  {
    if(eMode == MB_REG_READ)
    {
      if(__encode_modbus_register(slave, 0, reg_type, current_addr, &v) == FALSE)
      {
        TRACE(MAIN, "no channel mapping for %d:%d\n", reg_type, current_addr);
        return MB_ENORES;
      }
      U16_TO_BUFFER(v, buffer);
    }
    else
    {
      v = BUFFER_TO_U16(buffer);
      if(__decode_modbus_register(slave, 0, reg_type, current_addr, v) == FALSE)
      {
        TRACE(MAIN, "no channel mapping for %d:%d\n", reg_type, current_addr);
        return MB_ENORES;
      }
    }
  }
  return MB_ENOERR;
}

static MBErrorCode
modbus_slave_driver_input_regs_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                  uint16_t usAddress, uint16_t usNRegs)
{
  return __handle_modbus_u16(ctx, modbus_reg_input, pucRegBuffer, usAddress, usNRegs, MB_REG_READ);
}

static MBErrorCode
modbus_slave_driver_holding_regs_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                    uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  return __handle_modbus_u16(ctx, modbus_reg_holding, pucRegBuffer, usAddress, usNRegs, eMode);
}

static MBErrorCode
__handle_modbus_u1(ModbusSlaveCTX* ctx, modbus_reg_type_t reg_type, uint8_t* bufPtr,
    uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  uint16_t                current_addr,
                          end_addr;
  uint16_t                v,
                          bit_ndx;
  modbus_slave_driver_t   *slave;

  current_addr  = usAddress;
  end_addr      = current_addr + usNRegs - 1;

  slave = (modbus_slave_driver_t*)ctx->priv;

  for(bit_ndx = 0; current_addr <= end_addr; current_addr++, bit_ndx++)
  {
    if(eMode == MB_REG_READ)
    {
      if(__encode_modbus_register(slave, 0, reg_type, current_addr, &v) == FALSE)
      {
        TRACE(MAIN, "no channel mapping for %d:%d\n", reg_type, current_addr);
        return MB_ENORES;
      }
      xMBUtilSetBits(bufPtr, bit_ndx, 1, v == 0 ? 0 : 1);
    }
    else
    {
      v = xMBUtilGetBits(bufPtr, bit_ndx, 1);
      if(__decode_modbus_register(slave, 0, reg_type, current_addr, v) == FALSE)
      {
        TRACE(MAIN, "no channel mapping for %d:%d\n", reg_type, current_addr);
        return MB_ENORES;
      }
    }
  }
  return MB_ENOERR;
}

static MBErrorCode
modbus_slave_driver_coil_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress,
            uint16_t usNRegs, MBRegisterMode eMode)
{
  return __handle_modbus_u1(ctx, modbus_reg_coil, pucRegBuffer, usAddress, usNRegs, eMode);
}

static MBErrorCode
modbus_slave_driver_discrete_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                uint16_t usAddress, uint16_t usNRegs)
{
  return __handle_modbus_u1(ctx, modbus_reg_discrete, pucRegBuffer, usAddress, usNRegs, MB_REG_READ);
}


////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
////////////////////////////////////////////////////////////////////////////////
static modbus_slave_driver_t*
alloc_init_modbus_slave(modbus_slave_driver_config_t* cfg)
{
  modbus_slave_driver_t*    slave;
  ModbusSlaveCTX*           ctx;

  slave = malloc(sizeof(modbus_slave_driver_t));
  if(slave == NULL)
  {
    TRACE(MBS_DRIVER,"failed to alloc modbus_slave_driver_t\n");
    goto failed;
  }

  INIT_LIST_HEAD(&slave->le);

  slave->mb_type    = cfg->protocol;

  if(slave->mb_type == modbus_slave_driver_type_tcp)
  {
    ModbusTCPSlave* tcp_slave;

    tcp_slave = malloc(sizeof(ModbusTCPSlave));
    if(tcp_slave == NULL)
    {
      TRACE(MBS_DRIVER,"failed to alloc ModbusTCPSlave\n");
      goto failed;
    }

    ctx = &tcp_slave->ctx;
    modbus_tcp_slave_init(tcp_slave, cfg->address, cfg->tcp_port);
  }
  else
  {
    ModbusRTUSlave* rtu_slave;

    rtu_slave = malloc(sizeof(ModbusRTUSlave));
    if(rtu_slave == NULL)
    {
      TRACE(MBS_DRIVER,"failed to alloc ModbusRTUSlave\n");
      goto failed;
    }

    ctx = &rtu_slave->ctx;
    modbus_rtu_slave_init(rtu_slave, (uint8_t)cfg->address, cfg->serial_port, &cfg->serial_cfg);
  }

  ctx->input_regs_cb      = modbus_slave_driver_input_regs_cb;
  ctx->holding_regs_cb    = modbus_slave_driver_holding_regs_cb;
  ctx->coil_cb            = modbus_slave_driver_coil_cb;
  ctx->discrete_cb        = modbus_slave_driver_discrete_cb;

  slave->ctx = ctx;
  ctx->priv = (void*)slave;

  modbus_register_list_init(&slave->reg_map);
  list_add_tail(&slave->le, &_modbus_slaves);
  _num_modbus_slaves++;
  return slave;

failed:
  exit(-1);
  return NULL;
}

static void
modbus_slave_driver_load_slaves(void)
{
  modbus_slave_driver_config_t    cfg;
  int                             num_slaves,
                                  num_regs,
                                  i;
  modbus_slave_driver_t*          slave;

  num_slaves = cfg_mgr_get_num_modbus_slaves();

  // init phase
  for( i = 0; i < num_slaves; i++)
  {
    cfg_mgr_get_modbus_slave_at(i, &cfg);

    if(cfg.protocol == modbus_slave_driver_type_tcp)
    {
      TRACE(MBS_DRIVER,"initializing modbus tcp slave, port %d\n", cfg.tcp_port);
    }
    else
    {
      TRACE(MBS_DRIVER, "initializing modbus rtu slave, port %s\n", cfg.serial_port);
    }

    slave = alloc_init_modbus_slave(&cfg);

    // load registers
    num_regs = cfg_mgr_get_modbus_slave_num_regs(i);
    for(int reg_ndx = 0; reg_ndx < num_regs; reg_ndx++)
    {
      uint32_t            chnl;
      modbus_address_t    reg;
      modbus_reg_filter_t         filter;

      cfg_mgr_get_modbus_slave_reg(i, reg_ndx, &reg, &chnl, &filter);
      modbus_register_list_add(&slave->reg_map,
          reg.slave_id, reg.reg_type, reg.mb_address, chnl, &filter);
    }
  }

  // start phase
  list_for_each_entry(slave, &_modbus_slaves, le)
  {
    if(slave->mb_type == modbus_slave_driver_type_tcp)
    {
      modbus_tcp_slave_start((ModbusTCPSlave*)slave->ctx);
    }
    else
    {
      modbus_rtu_slave_start((ModbusRTUSlave*)slave->ctx);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// thread initializer
//
////////////////////////////////////////////////////////////////////////////////
static void
mb_slave_driver_thread_init(evloop_thread_t* thrd)
{
  TRACE(MBS_DRIVER, "loading slaves from config\n");
  modbus_slave_driver_load_slaves();

  TRACE(MBS_DRIVER, "done loading slaves\n");

  app_init_complete_signal();
}

static void
mb_slave_driver_thread_fini(evloop_thread_t* thrd)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
modbus_slave_driver_init(void)
{
  TRACE(MBS_DRIVER, "starting modbus slave driver\n");

  evloop_thread_init(&_mb_slave_driver_thread);
  evloop_thread_run(&_mb_slave_driver_thread);

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
  "slave_list" : [
    {
      "slave_type":           "tcp" or "rtu",
      "slave_address:"        xxx,
      "rx_frames":            xxx,
      "tx_frames" :           xxx,
      "my_frames" :           xxx,
      "req_fails" :           xxx,
      "rx_crc_error" :        xxx,
      "rx_buffer_overflow":   xxx,

      in case of tcp
      "bound_address":        xxx,
      "connections": [
        {
          "ip":   "xxx.xxx.xxx.xxx",
          "port": xxxx
        }
      ]
    }
  ]
 }
 */
cJSON*
modbus_slave_driver_get_stat(void)
{
  cJSON*                    ret;
  cJSON*                    jslave_list;
  modbus_slave_driver_t*    slave;

  evloop_thread_lock(&_mb_slave_driver_thread);

  ret = cJSON_CreateObject();

  jslave_list = cJSON_CreateArray();
  list_for_each_entry(slave, &_modbus_slaves, le)
  {
    if(slave->mb_type == modbus_slave_driver_type_tcp)
    {
      ModbusTCPSlave*   tcp_slave;

      tcp_slave = container_of(slave->ctx, ModbusTCPSlave, ctx);
      cJSON_AddItemToArray(jslave_list, modbus_tcp_slave_get_stat(tcp_slave));
    }
    else
    {
      ModbusRTUSlave*   rtu_slave;

      rtu_slave = container_of(slave->ctx, ModbusRTUSlave, ctx);
      cJSON_AddItemToArray(jslave_list, modbus_rtu_slave_get_stat(rtu_slave));
    }
  }

  cJSON_AddItemToObject(ret, "slave_list", jslave_list);

  evloop_thread_unlock(&_mb_slave_driver_thread);

  return ret;
}

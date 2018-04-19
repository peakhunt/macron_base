#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "common_def.h"
#include "app_modbus_slave.h"
#include "app_config.h"
#include "list.h"
#include "trace.h"
#include "channel_manager.h"
#include "modbus_rtu_slave.h"
#include "modbus_tcp_slave.h"
#include "modbus_util.h"
#include "modbus_regs.h"
#include "app_init_completion.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  struct list_head        le;
  app_modbus_slave_type_t mb_type;
  ModbusSlaveCTX          *ctx;
  modbus_register_list_t  reg_map;
} app_modbus_slave_t;

static void app_mb_slave_thread_init(evloop_thread_t* thrd);
static void app_mb_slave_thread_fini(evloop_thread_t* thrd);

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
static LIST_HEAD_DECL(_modbus_slaves);
static int _num_modbus_slaves = 0;

static evloop_thread_t    _app_mb_slave_thread =
{
  .init = app_mb_slave_thread_init,
  .fini = app_mb_slave_thread_fini,
};

////////////////////////////////////////////////////////////////////////////////
//
// common modbus request handlers
//
////////////////////////////////////////////////////////////////////////////////
static int32_t
__get_mapped_channel(app_modbus_slave_t* slave, uint32_t slave_id, modbus_reg_type_t reg_type, uint32_t addr)
{
  modbus_register_t*  reg;

  reg = modbus_register_list_lookup_by_mb_type_addr(&slave->reg_map,
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
// common modbus request handlers
//
////////////////////////////////////////////////////////////////////////////////
static MBErrorCode
__handle_modbus_u16(ModbusSlaveCTX* ctx, modbus_reg_type_t reg_type, uint8_t* bufPtr,
    uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  uint16_t            current_addr,
                      end_addr;
  uint8_t*            buffer;
  int32_t             chnl_num;
  uint16_t            v;
  app_modbus_slave_t  *app_slave;

  current_addr  = usAddress;
  end_addr      = current_addr + usNRegs - 1;
  buffer        = bufPtr;

  app_slave = (app_modbus_slave_t*)ctx->priv;

  for(; current_addr <= end_addr; current_addr++, buffer += 2)
  {
    chnl_num = __get_mapped_channel(app_slave, 0, reg_type, current_addr);
    if(chnl_num == -1)
    {
      TRACE(MAIN, "no channel mapping for %d:%d\n", reg_type, current_addr);
      return MB_ENORES;
    }

    if(eMode == MB_REG_READ)
    {
      v = (uint32_t)channel_manager_get_raw_value((uint32_t)chnl_num);
      U16_TO_BUFFER(v, buffer);
    }
    else
    {
      v = BUFFER_TO_U16(buffer);
      channel_manager_set_raw_value((uint32_t)chnl_num, (uint32_t)v);
    }
  }
  return MB_ENOERR;
}

static MBErrorCode
app_input_regs_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                  uint16_t usAddress, uint16_t usNRegs)
{
  return __handle_modbus_u16(ctx, modbus_reg_input, pucRegBuffer, usAddress, usNRegs, MB_REG_READ);
}

static MBErrorCode
app_holding_regs_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                    uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  return __handle_modbus_u16(ctx, modbus_reg_holding, pucRegBuffer, usAddress, usNRegs, eMode);
}

static MBErrorCode
__handle_modbus_u1(ModbusSlaveCTX* ctx, modbus_reg_type_t reg_type, uint8_t* bufPtr,
    uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  uint16_t            current_addr,
                      end_addr;
  int32_t             chnl_num;
  uint16_t            v,
                      bit_ndx;
  app_modbus_slave_t  *app_slave;

  current_addr  = usAddress;
  end_addr      = current_addr + usNRegs - 1;

  app_slave = (app_modbus_slave_t*)ctx->priv;

  for(bit_ndx = 0; current_addr <= end_addr; current_addr++, bit_ndx++)
  {
    chnl_num = __get_mapped_channel(app_slave, 0, reg_type, current_addr);
    if(chnl_num == -1)
    {
      TRACE(MAIN, "no channel mapping for %d:%d\n", reg_type, current_addr);
      return MB_ENORES;
    }

    if(eMode == MB_REG_READ)
    {
      v = channel_manager_get_raw_value((uint32_t)chnl_num);
      xMBUtilSetBits(bufPtr, bit_ndx, 1, v == 0 ? 0 : 1);
    }
    else
    {
      v = xMBUtilGetBits(bufPtr, bit_ndx, 1);
      channel_manager_set_raw_value((uint32_t)chnl_num, (uint32_t)v);
    }
  }
  return MB_ENOERR;
}

static MBErrorCode
app_coil_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress,
            uint16_t usNRegs, MBRegisterMode eMode)
{
  return __handle_modbus_u1(ctx, modbus_reg_coil, pucRegBuffer, usAddress, usNRegs, eMode);
}

static MBErrorCode
app_discrete_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                uint16_t usAddress, uint16_t usNRegs)
{
  return __handle_modbus_u1(ctx, modbus_reg_discrete, pucRegBuffer, usAddress, usNRegs, MB_REG_READ);
}


////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
////////////////////////////////////////////////////////////////////////////////
static app_modbus_slave_t*
alloc_init_modbus_slave(app_modbus_slave_config_t* cfg)
{
  app_modbus_slave_t*   slave;
  ModbusSlaveCTX*       ctx;

  slave = malloc(sizeof(app_modbus_slave_t));
  if(slave == NULL)
  {
    TRACE(APP_MB_SLAVE,"failed to alloc app_modbus_slave_t\n");
    goto failed;
  }

  INIT_LIST_HEAD(&slave->le);

  slave->mb_type    = cfg->protocol;

  if(slave->mb_type == app_modbus_slave_type_tcp)
  {
    ModbusTCPSlave* tcp_slave;

    tcp_slave = malloc(sizeof(ModbusTCPSlave));
    if(tcp_slave == NULL)
    {
      TRACE(APP_MB_SLAVE,"failed to alloc ModbusTCPSlave\n");
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
      TRACE(APP_MB_SLAVE,"failed to alloc ModbusRTUSlave\n");
      goto failed;
    }

    ctx = &rtu_slave->ctx;
    modbus_rtu_slave_init(rtu_slave, (uint8_t)cfg->address, cfg->serial_port, &cfg->serial_cfg);
  }

  ctx->input_regs_cb      = app_input_regs_cb;
  ctx->holding_regs_cb    = app_holding_regs_cb;
  ctx->coil_cb            = app_coil_cb;
  ctx->discrete_cb        = app_discrete_cb;

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
app_modbus_load_slaves(void)
{
  app_modbus_slave_config_t   cfg;
  int                         num_slaves,
                              num_regs,
                              i;
  app_modbus_slave_t*         slave;

  num_slaves = app_config_get_num_modbus_slaves();

  // init phase
  for( i = 0; i < num_slaves; i++)
  {
    app_config_get_modbus_slave_at(i, &cfg);

    if(cfg.protocol == app_modbus_slave_type_tcp)
    {
      TRACE(APP_MB_SLAVE,"initializing modbus tcp slave, port %d\n", cfg.tcp_port);
    }
    else
    {
      TRACE(APP_MB_SLAVE, "initializing modbus rtu slave, port %s\n", cfg.serial_port);
    }

    slave = alloc_init_modbus_slave(&cfg);

    // load registers
    num_regs = app_config_get_modbus_slave_num_regs(i);
    for(int reg_ndx = 0; reg_ndx < num_regs; reg_ndx++)
    {
      uint32_t            chnl;
      modbus_address_t    reg;

      app_config_get_modbus_slave_reg(i, reg_ndx, &reg, &chnl);
      modbus_register_list_add(&slave->reg_map,
          reg.slave_id, reg.reg_type, reg.mb_address, chnl);
    }
  }

  // start phase
  list_for_each_entry(slave, &_modbus_slaves, le)
  {
    if(slave->mb_type == app_modbus_slave_type_tcp)
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
app_mb_slave_thread_init(evloop_thread_t* thrd)
{
  TRACE(APP_MB_SLAVE, "loading slaves from config\n");
  app_modbus_load_slaves();

  TRACE(APP_MB_SLAVE, "done loading slaves\n");

  app_init_complete_signal();
}

static void
app_mb_slave_thread_fini(evloop_thread_t* thrd)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
app_modbus_slave_init(void)
{
  TRACE(APP_MB_SLAVE, "starting modbus slave driver\n");

  evloop_thread_init(&_app_mb_slave_thread);
  evloop_thread_run(&_app_mb_slave_thread);

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
app_api_modbus_slave_get_stat(void)
{
  cJSON*                ret;
  cJSON*                jslave_list;
  app_modbus_slave_t*   slave;

  evloop_thread_lock(&_app_mb_slave_thread);

  ret = cJSON_CreateObject();

  jslave_list = cJSON_CreateArray();
  list_for_each_entry(slave, &_modbus_slaves, le)
  {
    if(slave->mb_type == app_modbus_slave_type_tcp)
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

  evloop_thread_unlock(&_app_mb_slave_thread);

  return ret;
}

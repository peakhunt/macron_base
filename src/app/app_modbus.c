#include <stdio.h>
#include <stdlib.h>
#include "common_def.h"
#include "app_modbus.h"
#include "app_config.h"
#include "list.h"
#include "trace.h"
#include "io_channel_map.h"
#include "channel_manager.h"
#include "modbus_rtu_slave.h"
#include "modbus_tcp_slave.h"
#include "modbus_util.h"

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
  int                     reg_group;
} app_modbus_slave_t;

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
LIST_HEAD(_modbus_slaves);

////////////////////////////////////////////////////////////////////////////////
//
// common modbus request handlers
//
////////////////////////////////////////////////////////////////////////////////
static int32_t
__get_mapped_channel(int group, modbus_reg_type_t reg_type, uint32_t addr)
{
  modbus_address_t    mb_addr;

  mb_addr.mb_address    = addr;
  mb_addr.reg_type      = reg_type;

  return io_channel_map_lookup_modbus(group, &mb_addr);
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
    chnl_num = __get_mapped_channel(app_slave->reg_group, reg_type, current_addr);
    if(chnl_num == -1)
    {
      TRACE(MAIN, "no channel mapping for %d:%d:%d\n", app_slave->reg_group, reg_type, current_addr);
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
    chnl_num = __get_mapped_channel(app_slave->reg_group, reg_type, current_addr);
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
static void
alloc_init_modbus_slave(app_modbus_slave_config_t* cfg)
{
  app_modbus_slave_t*   slave;
  ModbusSlaveCTX*       ctx;

  slave = malloc(sizeof(app_modbus_slave_t));
  if(slave == NULL)
  {
    TRACE(APP_START,"failed to alloc app_modbus_slave_t\n");
      goto failed;
  }

  INIT_LIST_HEAD(&slave->le);

  slave->mb_type    = cfg->protocol;
  slave->reg_group  = cfg->reg_group;

  if(slave->mb_type == app_modbus_slave_type_tcp)
  {
    ModbusTCPSlave* tcp_slave;

    tcp_slave = malloc(sizeof(ModbusTCPSlave));
    if(tcp_slave == NULL)
    {
      TRACE(APP_START,"failed to alloc ModbusTCPSlave\n");
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
      TRACE(APP_START,"failed to alloc ModbusRTUSlave\n");
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
  
  list_add_tail(&slave->le, &_modbus_slaves);
  return;

failed:
  exit(-1);
}

static void
app_modbus_load_slaves(void)
{
  app_modbus_slave_config_t   cfg;
  int                         num_slaves,
                              i;
  app_modbus_slave_t*         slave;

  num_slaves = app_config_get_num_modbus_slaves();

  // init phase
  for( i = 0; i < num_slaves; i++)
  {
    app_config_get_modbus_slaves_at(i, &cfg);

    if(cfg.protocol == app_modbus_slave_type_tcp)
    {
      TRACE(APP_START,"initializing modbus tcp slave, port %d\n", cfg.tcp_port);
    }
    else
    {
      TRACE(APP_START, "initializing modbus rtu slave, port %s\n", cfg.serial_port);
    }
    alloc_init_modbus_slave(&cfg);
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
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
app_modbus_init(void)
{
  app_modbus_load_slaves();
}

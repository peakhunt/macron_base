#include <stdio.h>
#include <stdlib.h>
#include "app_modbus.h"
#include "app_config.h"
#include "list.h"
#include "trace.h"

#include "modbus_rtu_slave.h"
#include "modbus_tcp_slave.h"

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
static MBErrorCode
app_input_regs_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                  uint16_t usAddress, uint16_t usNRegs)
{
  return MB_ENOERR;
}

static MBErrorCode
app_holding_regs_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                    uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode)
{
  return MB_ENOERR;
}

static MBErrorCode
app_coil_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress,
            uint16_t usNRegs, MBRegisterMode eMode)
{
  return MB_ENOERR;
}

static MBErrorCode
app_discrete_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                uint16_t usAddress, uint16_t usNRegs)
{
  return MB_ENOERR;
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

  slave->mb_type = cfg->protocol;

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
  
  list_add_tail(&slave->le, &_modbus_slaves);
  return;

failed:
  exit(-1);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
app_modbus_init(void)
{
  app_modbus_slave_config_t   cfg;
  int   num_slaves,
        i;

  app_modbus_slave_t*   slave;

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

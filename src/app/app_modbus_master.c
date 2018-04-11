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
#include "trace.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  struct list_head          le;
  app_modbus_master_type_t  mb_type;
  ModbusMasterCTX*          ctx;
  modbus_register_list_t    reg_map;
  int                       current_request;
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

  // FIXME
  // ctx callbacks
  // FIXME

  master->ctx = ctx;
  ctx->priv   = (void*)master;

  modbus_register_list_init(&master->reg_map);
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

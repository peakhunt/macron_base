#ifndef __APP_MODBUS_MASTER_DEF_H__
#define __APP_MODBUS_MASTER_DEF_H__

#include "common_def.h"
#include "serial.h"
#include "modbus_regs.h"
#include "cJSON.h"

typedef enum
{
  app_modbus_master_type_tcp,
  app_modbus_master_type_rtu
} app_modbus_master_type_t;

typedef struct
{
  app_modbus_master_type_t    protocol;
  uint32_t                    schedule_finish_delay;
  uint32_t                    inter_request_delay;
  uint32_t                    timeout;
  char*                       dest_ip;
  int                         dest_port;
  char*                       serial_port;
  SerialConfig                serial_cfg;
} app_modbus_master_config_t;

typedef struct
{
  int                 slave_id;
  modbus_reg_op_t     op;
  modbus_reg_type_t   reg;
  int                 start_addr;
  int                 num_regs;
} app_modbus_master_request_config_t;

extern void app_modbus_master_init(void);

////////////////////////////////////////////////////////////
//
// module APIs
//
////////////////////////////////////////////////////////////
extern cJSON* app_api_modbus_master_get_stat(void);

#endif /* !__APP_MODBUS_MASTER_DEF_H__ */

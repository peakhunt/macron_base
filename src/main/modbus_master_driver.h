#ifndef __MODBUS_MASTER_DRIVER_DEF_H__
#define __MODBUS_MASTER_DRIVER_DEF_H__

#include "common_def.h"
#include "serial.h"
#include "modbus_regs.h"
#include "cJSON.h"

typedef enum
{
  modbus_master_driver_type_tcp,
  modbus_master_driver_type_rtu
} modbus_master_driver_type_t;

typedef struct
{
  modbus_master_driver_type_t protocol;
  uint32_t                    schedule_finish_delay;
  uint32_t                    inter_request_delay;
  uint32_t                    timeout;
  char*                       dest_ip;
  int                         dest_port;
  char*                       serial_port;
  SerialConfig                serial_cfg;
} modbus_master_driver_config_t;

typedef struct
{
  int                 slave_id;
  modbus_reg_op_t     op;
  modbus_reg_type_t   reg;
  int                 start_addr;
  int                 num_regs;
} modbus_master_driver_request_config_t;

extern void modbus_master_driver_init(void);

////////////////////////////////////////////////////////////
//
// module APIs
//
////////////////////////////////////////////////////////////
extern cJSON* modbus_master_driver_get_stat(void);

#endif /* !__MODBUS_MASTER_DRIVER_DEF_H__ */

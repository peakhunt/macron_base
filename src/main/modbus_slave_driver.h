#ifndef __MODBUS_SLAVE_DRIVER_DEF_H__
#define __MODBUS_SLAVE_DRIVER_DEF_H__

#include "common_def.h"
#include "serial.h"
#include "modbus_regs.h"
#include "cJSON.h"

typedef enum
{
  modbus_slave_driver_type_tcp,
  modbus_slave_driver_type_rtu
} modbus_slave_driver_type_t;

typedef struct
{
  modbus_slave_driver_type_t  protocol;
  uint16_t                    address;
  int                         tcp_port;
  char*                       serial_port;
  SerialConfig                serial_cfg;
} modbus_slave_driver_config_t;

extern void modbus_slave_driver_init(void);

////////////////////////////////////////////////////////////
//
// module APIs
//
////////////////////////////////////////////////////////////
extern cJSON* modbus_slave_driver_get_stat(void);

#endif /* !__MODBUS_SLAVE_DRIVER_DEF_H__ */

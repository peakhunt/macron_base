#ifndef __APP_MODBUS_DEF_H__
#define __APP_MODBUS_DEF_H__

#include "common_def.h"
#include "serial.h"
#include "modbus_regs.h"

typedef enum
{
  app_modbus_slave_type_tcp,
  app_modbus_slave_type_rtu,
} app_modbus_slave_type_t;

typedef struct
{
  app_modbus_slave_type_t     protocol;
  uint16_t                    address;
  int                         tcp_port;
  char*                       serial_port;
  SerialConfig                serial_cfg;
  int                         reg_group;
} app_modbus_slave_config_t;

extern void app_modbus_init(void);

#endif /* !__APP_MODBUS_DEF_H__ */

#ifndef __APP_MODBUS_SLAVE_DEF_H__
#define __APP_MODBUS_SLAVE_DEF_H__

#include "common_def.h"
#include "serial.h"
#include "modbus_regs.h"

typedef enum
{
  app_modbus_slave_type_tcp,
  app_modbus_slave_type_rtu
} app_modbus_slave_type_t;

typedef struct
{
  app_modbus_slave_type_t     protocol;
  uint16_t                    address;
  int                         tcp_port;
  char*                       serial_port;
  SerialConfig                serial_cfg;
} app_modbus_slave_config_t;

extern void app_modbus_slave_init(void);

////////////////////////////////////////////////////////////
//
// module APIs
//
////////////////////////////////////////////////////////////
extern int app_api_modbus_slave_get_num_slaves(void);
extern int app_api_modbus_slave_get_stat(int slave_ndx);

#endif /* !__APP_MODBUS_SLAVE_DEF_H__ */

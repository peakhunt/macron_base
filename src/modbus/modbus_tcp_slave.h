#ifndef __MODBUS_TCP_SLAVE_DEF_H__
#define __MODBUS_TCP_SLAVE_DEF_H__

#include <stdint.h>
#include "watcher.h"

#include "modbus_slave.h"

#include "list.h"
#include "tcp_server.h"
#include "tcp_server_ipv4.h"
#include "cJSON.h"

/*
 *
 *  <------------------------ MODBUS TCP LINE PDU (1) ------->
 *              <----------- MODBUS PDU (1') ---------------->
 *  +-----------+---------------+----------------------------+
 *  | MBAP Head | Function Code | Data                       | 
 *  +-----------+---------------+----------------------------+
 *  |           |               |                                   |
 * (2)        (3/2')           (3')                                (4)
 *
 * (1)  ... MB_TCP_PDU_SIZE_MAX = 260
 * (2)  ... MB_TCP_PDU_ADDR_OFF = 6
 * (3)  ... MB_TCP_PDU_PDU_OFF  = 7
 * (4)  ... MB_TCP_PDU_SIZE_CRC = NONE
 */

#define MB_TCP_PDU_SIZE_MIN           7
#define MB_TCP_PDU_SIZE_MAX           260

#define MB_TCP_PDU_TID_OFF            0
#define MB_TCP_PDU_PID_OFF            2
#define MB_TCP_PDU_LEN_OFF            4
#define MB_TCP_PDU_ADDR_OFF           6

#define MB_TCP_PDU_OFF                7

typedef struct
{
  ModbusSlaveCTX        ctx;
  uint8_t               my_address;

  tcp_server_t          server;
  struct list_head      connections;      // list of connected modbus masters
} ModbusTCPSlave;

extern void modbus_tcp_slave_init(ModbusTCPSlave* slave, uint8_t device_addr, int tcp_port);
extern void modbus_tcp_slave_start(ModbusTCPSlave* slave);
extern void modbus_tcp_slave_stop(ModbusTCPSlave* slave);

extern cJSON* modbus_tcp_slave_get_stat(ModbusTCPSlave* tcp_slave);
extern cJSON* modbus_tcp_slave_get_connctions(ModbusTCPSlave* tcp_slave);

#endif /* !__MODBUS_TCP_SLAVE_DEF_H__ */

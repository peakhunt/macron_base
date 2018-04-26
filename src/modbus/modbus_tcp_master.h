#ifndef __MODBUS_TCP_MASTER_DEF_H__
#define __MODBUS_TCP_MASTER_DEF_H__

#include <stdint.h>
#include "tcp_auto_connector.h"
#include "stream.h"
#include "mbap_reader.h"
#include "modbus_master.h"
#include "cJSON.h"

typedef enum
{
  ModbusTCPMasterState_Not_Connected,
  ModbusTCPMasterState_Connecting,
  ModbusTCPMasterState_Connected,
} ModbusTCPMasterState_t;

typedef struct
{
  ModbusMasterCTX         ctx;
  uint8_t                 rx_bounce_buf[128];

  struct sockaddr_in      server_addr;
  tcp_auto_connector_t    tcp_connector;
  ModbusTCPMasterState_t  tcp_state;

  stream_t                stream;

  uint16_t                tid;
  uint16_t                pid;

  mbap_reader_t           mbap_reader;
  uint32_t                n_disconnect;
} ModbusTCPMaster;

extern void modbus_tcp_master_init(ModbusTCPMaster* master, struct sockaddr_in*  server_addr);
extern void modbus_tcp_master_init_with_ip_port(ModbusTCPMaster* master, const char* dest_ip, int dest_port);
extern void modbus_tcp_master_start(ModbusTCPMaster* master);
extern void modbus_tcp_master_stop(ModbusTCPMaster* master);

extern cJSON* modbus_tcp_master_get_stat(ModbusTCPMaster* master);

#endif /* !__MODBUS_TCP_MASTER_DEF_H__ */

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "common_def.h"
#include "modbus_slave.h"
#include "modbus_tcp_slave.h"
#include "modbus_crc.h"
#include "modbus_rtu_request_handler.h"
#include "modbus_funcs.h"
#include "mbap_reader.h"

#include "stream.h"
#include "sock_util.h"
#include "trace.h"
#include "hex_dump.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// internal definitions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
  struct list_head      le;
  stream_t              stream;
  ModbusTCPSlave*       slave;
  uint8_t               rx_buf[512];

  struct sockaddr_in    from;

  mbap_reader_t         mbap_reader;
} modbus_tcp_slave_connection_t;

static void modbus_tcp_slave_handle_stream_event(stream_t* stream, stream_event_t evt);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MODBUS TCP Frame Handling Core
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_handle_rx_frame(ModbusTCPSlave* slave, modbus_tcp_slave_connection_t* conn)
{
  uint8_t             *pdu,
                      ret;
  uint16_t            len,
                      rsp_len;
  ModbusSlaveCTX*          ctx = &slave->ctx;
  mbap_reader_t*      mbap = &conn->mbap_reader;

  TRACE_DUMP(MB_TCP_SLAVE, "MB TCP Slave RX", mbap->frame, mbap->ndx);

  pdu     = &conn->mbap_reader.frame[MB_TCP_PDU_OFF];
  len     = mbap->length - 1;

  if(mbap->uid == slave->my_address || mbap->uid == MB_ADDRESS_BROADCAST)
  {
    ret = modbus_rtu_handler_request_rx(&slave->ctx, mbap->uid, len, pdu, &rsp_len);
    if(mbap->uid != MB_ADDRESS_BROADCAST)
    {
      ctx->my_frames++;
      if(ret != TRUE)
      {
        ctx->req_fails++;
      }

      // taking unit ID into account
      rsp_len += 1;

      //
      // finish TX frame for RTU slave by updating response length
      //
      mbap->frame[4]  = (uint8_t)(rsp_len >> 8 & 0xff);
      mbap->frame[5]  = (uint8_t)(rsp_len >> 0 & 0xff);

      rsp_len += 6;   // mbap header

      TRACE_DUMP(MB_TCP_SLAVE, "MB TCP Slave TX", mbap->frame, rsp_len);

      if(stream_write(&conn->stream, &mbap->frame[0], rsp_len) == FALSE)
      {
        TRACE(MB_TCP_SLAVE, "tx failed\n");
      }
    }
  }
  else
  {
    TRACE(MB_TCP_SLAVE, "address mismatch mine: %d, requested: %d\n", slave->my_address, mbap->uid);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MBAP Reader Callback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_got_frame(mbap_reader_t* mbap)
{
  modbus_tcp_slave_connection_t*  conn  = container_of(mbap, modbus_tcp_slave_connection_t, mbap_reader);
  ModbusTCPSlave*                 slave = conn->slave;

  modbus_tcp_slave_handle_rx_frame(slave, conn);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// connection alloc/dealloc
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_alloc_conn(ModbusTCPSlave* slave, int newsd, struct sockaddr_in* from)
{
  modbus_tcp_slave_connection_t*    conn;

  TRACE(MB_TCP_SLAVE, "got new connection\n");

  conn = malloc(sizeof(modbus_tcp_slave_connection_t));
  if(conn == NULL)
  {
    TRACE(MB_TCP_SLAVE, "connection alloc failed\n");
    close(newsd);
    return;
  }

  conn->slave = slave;

  INIT_LIST_HEAD(&conn->le);

  memcpy(&conn->from, from, sizeof(struct sockaddr_in));

  stream_init_with_fd(&conn->stream, newsd, conn->rx_buf, 512, 512);
  sock_util_put_nonblock(newsd);

  list_add_tail(&conn->le,  &slave->connections);

  mbap_reader_init(&conn->mbap_reader, modbus_tcp_slave_got_frame);

  conn->stream.cb   = modbus_tcp_slave_handle_stream_event;
  stream_start(&conn->stream);
}

static void
modbus_tcp_slave_dealloc_conn(modbus_tcp_slave_connection_t* conn)
{
  stream_deinit(&conn->stream);
  list_del_init(&conn->le);
  free(conn);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// stream callback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_handle_stream_event(stream_t* stream, stream_event_t evt)
{
  modbus_tcp_slave_connection_t* conn = container_of(stream, modbus_tcp_slave_connection_t, stream);

  switch(evt)
  {
  case stream_event_rx:
    mbap_reader_feed(&conn->mbap_reader, stream->rx_buf, stream->rx_data_len);
    break;

  case stream_event_eof:
    TRACE(MB_TCP_SLAVE, "connection eof detected\n");
    modbus_tcp_slave_dealloc_conn(conn);
    break;

  case stream_event_err:
    TRACE(MB_TCP_SLAVE, "connection error detected\n");
    modbus_tcp_slave_dealloc_conn(conn);
    break;

  case stream_event_tx:
    // never occurs
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// new connection callback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_slave_new_connection(tcp_server_t* server, int newsd, struct sockaddr* from)
{
  ModbusTCPSlave* slave = container_of(server, ModbusTCPSlave, server);

  modbus_tcp_slave_alloc_conn(slave, newsd, (struct sockaddr_in*)from);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
modbus_tcp_slave_init(ModbusTCPSlave* slave, uint8_t device_addr, int tcp_port)
{
  mb_slave_ctx_init(&slave->ctx);

  slave->my_address = device_addr;

  slave->server.conn_cb = modbus_tcp_slave_new_connection;
  tcp_server_ipv4_init(&slave->server, tcp_port, 5);

  INIT_LIST_HEAD(&slave->connections);
}

void
modbus_tcp_slave_start(ModbusTCPSlave* slave)
{
  tcp_server_start(&slave->server);
}

void
modbus_tcp_slave_stop(ModbusTCPSlave* slave)
{
  modbus_tcp_slave_connection_t   *p, *n;

  tcp_server_stop(&slave->server);

  list_for_each_entry_safe(p, n, &slave->connections, le)
  {
    modbus_tcp_slave_dealloc_conn(p);
  }
}

/**
 {
  "slave_type":     "tcp",
  "slave_address":  "xxx,
  "rx_frames":            xxx,
  "tx_frames" :           xxx,
  "my_frames" :           xxx,
  "req_fails" :           xxx,
  "rx_crc_error" :        xxx,
  "rx_buffer_overflow":   xxx,
  "bound_address":        "xxx:xxx",
  "connections": [
    {
      "ip": "xxx.xxx.xxx.xxx",
      "port": xxx
    }
    ...
  ]
 }
 */
cJSON*
modbus_tcp_slave_get_stat(ModbusTCPSlave* tcp_slave)
{
  cJSON*    jslave;
  char      name[MAX_ADDRESS_STRING_LEN];

  jslave = cJSON_CreateObject();

  cJSON_AddItemToObject(jslave, "slave_type",cJSON_CreateString("tcp"));
  cJSON_AddItemToObject(jslave, "slave_address", cJSON_CreateNumber(tcp_slave->my_address));
  mb_slave_ctx_get_stat(&tcp_slave->ctx, jslave);

  tcp_server_get_port_name(&tcp_slave->server, name);
  cJSON_AddItemToObject(jslave, "bound_address", cJSON_CreateString(name));
  cJSON_AddItemToObject(jslave, "connections", modbus_tcp_slave_get_connctions(tcp_slave));

  return jslave;
}

/**
  [
    {
      "ip": "xxx.xxx.xxx.xxx",
      "port": xxx,
    },
    ...
  ]
 */
cJSON*
modbus_tcp_slave_get_connctions(ModbusTCPSlave* tcp_slave)
{
  modbus_tcp_slave_connection_t* p;
  cJSON*  ret;
  cJSON*  item;

  ret = cJSON_CreateArray();

  list_for_each_entry(p, &tcp_slave->connections,  le)
  {
    item = cJSON_CreateObject();

    cJSON_AddItemToObject(item, "ip", cJSON_CreateString(inet_ntoa(p->from.sin_addr)));
    cJSON_AddItemToObject(item, "port", cJSON_CreateNumber(ntohs(p->from.sin_port)));

    cJSON_AddItemToArray(ret, item);
  }

  return ret;
}

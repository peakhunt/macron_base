#include <unistd.h>
#include "common_def.h"
#include "trace.h"
#include "modbus_tcp_master.h"
#include "modbus_rtu_response_handler.h"

#define MODBUS_TCP_MASTER_CONNECT_TIMEOUT       5.0
#define MODBUS_TCP_MASTER_PROBATION_TIMEOUT     1.0

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_master_start_reconnect(ModbusTCPMaster* master)
{
  stream_deinit(&master->stream);
  mbap_reader_reset(&master->mbap_reader);

  master->tcp_state   = ModbusTCPMasterState_Connecting;

  tcp_auto_connector_init(&master->tcp_connector, &master->server_addr,
      MODBUS_TCP_MASTER_CONNECT_TIMEOUT,
      MODBUS_TCP_MASTER_PROBATION_TIMEOUT);
  tcp_auto_connector_start(&master->tcp_connector);
}

static void
modbus_tcp_master_request(ModbusMasterCTX* ctx, uint8_t slave)
{
  ModbusTCPMaster*  master = container_of(ctx, ModbusTCPMaster, ctx);
  uint16_t          frame_len = ctx->tx_ndx + 1 - 7;    // + slave

  if(master->tcp_state != ModbusTCPMasterState_Connected)
  {
    TRACE(MB_TCP_MASTER, "modbus request when TCP not connected \n");
    return;
  }

  ctx->tx_buf[0]    = (uint8_t)(master->tid >> 8);
  ctx->tx_buf[1]    = (uint8_t)(master->tid & 0xff);
  ctx->tx_buf[2]    = (uint8_t)(master->pid >> 8);
  ctx->tx_buf[3]    = (uint8_t)(master->pid & 0xff);
  ctx->tx_buf[4]    = (uint8_t)(frame_len >> 8);
  ctx->tx_buf[5]    = (uint8_t)(frame_len & 0xff);
  ctx->tx_buf[6]    = slave;

  TRACE_DUMP(MB_TCP_MASTER, "MB TCP Master TX", ctx->tx_buf, ctx->tx_ndx);

  if(stream_write(&master->stream, ctx->tx_buf, ctx->tx_ndx) == FALSE)
  {
    TRACE(MB_TCP_MASTER, "tx error\n");
    if(master->ctx.event_cb != NULL)
    {
      master->ctx.event_cb(&master->ctx, modbus_master_event_disconnected);
    }
    modbus_tcp_master_start_reconnect(master);
  }

  master->tid++;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// internal tcp stream management
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_master_stream_callback(stream_t* stream, stream_event_t evt)
{
  ModbusTCPMaster*  master = container_of(stream, ModbusTCPMaster, stream);

  switch(evt)
  {
  case stream_event_rx:
    mbap_reader_feed(&master->mbap_reader, stream->rx_buf, stream->rx_data_len);
    break;

  case stream_event_eof:
  case stream_event_err:
    TRACE(MB_TCP_MASTER, "got eof/err event %d\n", evt);
    if(master->ctx.event_cb != NULL)
    {
      master->ctx.event_cb(&master->ctx, modbus_master_event_disconnected);
    }
    modbus_tcp_master_start_reconnect(master);
    break;

  default:
    TRACE(MB_TCP_MASTER, "stream_event : %d\n", evt);
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MBAP Reader Callback
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_master_got_frame(mbap_reader_t* mbap)
{
  ModbusTCPMaster* master = container_of(mbap, ModbusTCPMaster, mbap_reader);
  uint16_t    expected;

  TRACE_DUMP(MB_TCP_MASTER, "MB TCP Master RX", mbap->frame, mbap->ndx);

  expected = master->tid - 1;

  if(expected != mbap->tid)
  {
    TRACE(MB_TCP_MASTER, "TID Mismatch %d, %d\n", expected, mbap->tid);
    return;
  }

  if(master->pid != mbap->pid)
  {
    TRACE(MB_TCP_MASTER, "PID Mismatch %d, %d\n", master->pid, mbap->pid);
    return;
  }

  modbus_rtu_handler_response_rx(&master->ctx, mbap->uid, &mbap->frame[7], (int)mbap->length - 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// internal tcp connection management
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_tcp_master_tcp_connector_callback(tcp_auto_connector_t* con, int sd)
{
  ModbusTCPMaster* master = container_of(con, ModbusTCPMaster, tcp_connector);

  if(sd >= 0)
  {
    TRACE(MB_TCP_MASTER, "Connect success\n");

    master->tcp_state   = ModbusTCPMasterState_Connected;

    master->stream.cb    = modbus_tcp_master_stream_callback;
    stream_init_with_fd(&master->stream, sd, master->rx_bounce_buf,  128, 512);
    stream_start(&master->stream);
    
    mbap_reader_init(&master->mbap_reader, modbus_tcp_master_got_frame);

    if(master->ctx.event_cb != NULL)
    {
      master->ctx.event_cb(&master->ctx, modbus_master_event_connected);
    }
  }
  else
  {
    TRACE(MB_TCP_MASTER, "Connect failure\n");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
modbus_tcp_master_init_with_ip_port(ModbusTCPMaster* master, const char* dest_ip, int dest_port)
{
  struct sockaddr_in    server_addr;

  server_addr.sin_family       = AF_INET;
  server_addr.sin_addr.s_addr  = inet_addr(dest_ip);
  server_addr.sin_port         = htons(dest_port);

  modbus_tcp_master_init(master, &server_addr);
}

void
modbus_tcp_master_init(ModbusTCPMaster* master, struct sockaddr_in*  server_addr)
{
  master->ctx.pdu_offset    = 7;
  master->ctx.request       = modbus_tcp_master_request;
  mb_master_ctx_init(&master->ctx);

  master->tid   = 0;
  master->pid   = 0;

  memcpy(&master->server_addr, server_addr, sizeof(struct sockaddr_in));
  master->tcp_state   = ModbusTCPMasterState_Not_Connected;

  master->tcp_connector.cb = modbus_tcp_master_tcp_connector_callback;
  tcp_auto_connector_init(&master->tcp_connector, server_addr,
      MODBUS_TCP_MASTER_CONNECT_TIMEOUT,
      MODBUS_TCP_MASTER_PROBATION_TIMEOUT);
}

void
modbus_tcp_master_start(ModbusTCPMaster* master)
{
  master->tcp_state = ModbusTCPMasterState_Connecting;
  tcp_auto_connector_start(&master->tcp_connector);
}

void
modbus_tcp_master_stop(ModbusTCPMaster* master)
{
  switch(master->tcp_state)
  {
  case ModbusTCPMasterState_Not_Connected:
    // nothing to do
    break;

  case ModbusTCPMasterState_Connecting:
    tcp_auto_connector_stop(&master->tcp_connector);
    break;

  case ModbusTCPMasterState_Connected:
    stream_deinit(&master->stream);
    break;
  }

  master->tcp_state   = ModbusTCPMasterState_Not_Connected;
}

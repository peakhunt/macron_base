#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "common_def.h"
#include "modbus_master.h"
#include "modbus_rtu_master.h"
#include "modbus_crc.h"

#include "hex_dump.h"
#include "trace.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TX handling
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_rtu_master_request(ModbusMasterCTX* ctx, uint8_t slave)
{
  ModbusRTUMaster*  master = container_of(ctx, ModbusRTUMaster, ctx);
  uint16_t    crc;

  ctx->tx_buf[0]    = slave;

  crc = modbus_calc_crc(ctx->tx_buf, ctx->tx_ndx);

  ctx->tx_buf[ctx->tx_ndx++] = (uint8_t)(crc & 0xff);
  ctx->tx_buf[ctx->tx_ndx++] = (uint8_t)(crc >> 8);

  TRACE_DUMP(MB_RTU_MASTER, "MB RTU Master TX", ctx->tx_buf, ctx->tx_ndx);

  if(stream_write(&master->stream, ctx->tx_buf, ctx->tx_ndx) == FALSE)
  {
    TRACE(MB_RTU_MASTER, "tx error\n");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// RX handling
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_rtu_rx(ModbusRTUMaster* master)
{
  //
  // FIXME
  //
}

static void
modbus_rtu_master_stream_callback(stream_t* stream, stream_event_t evt)
{
  ModbusRTUMaster*  master = container_of(stream, ModbusRTUMaster, stream);

  switch(evt)
  {
  case stream_event_rx:
    modbus_rtu_rx(master);
    break;

  default:
    TRACE(MB_RTU_MASTER, "stream_event : %d\n", evt);
    break;
  }
}

static void
mb_rtu_start_handling_rx_frame(ModbusRTUMaster* master)
{
#if 0
  ModbusMasterCTX*    ctx = &master->ctx;

  //ctx->rx_frames++;

  //TRACE_DUMP(MB_RTU_MASTER, "MB RTU Master RX", master->data_buffer, master->data_ndx);

  if(mb_rtu_rx_len(slave) == 0)
  {
    // buffer overflow due to long frame has just occurred.
    TRACE(MB_RTU_SLAVE, "rx error len 0\n");
    mb_rtu_enable_rx(slave);
    return;
  }

  if(mb_rtu_rx_len(slave) < MB_SER_RTU_PDU_SIZE_MIN)
  {
    TRACE(MB_RTU_SLAVE, "rx error short frame 0\n");
    INC_ERR_CNT(slave->rx_short_frame);
    mb_rtu_enable_rx(slave);
    return;
  }

  if(modbus_calc_crc((uint8_t*)mb_rtu_buffer(slave), mb_rtu_rx_len(slave)) != 0)
  {
    TRACE(MB_RTU_SLAVE, "rx error crc error\n");
    // crc16 error
    INC_ERR_CNT(ctx->rx_crc_error);
    mb_rtu_enable_rx(slave);
    return;
  }

  mb_rtu_handle_rx_frame(slave);
#endif
}

static void
t35_timeout_handler(evloop_timer_t* te, void* unused)
{
  ModbusRTUMaster* master = container_of(te, ModbusRTUMaster, t35);

  mb_rtu_start_handling_rx_frame(master);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
modbus_rtu_master_init(ModbusRTUMaster* master, int fd)
{
  master->ctx.pdu_offset    = 1;
  master->ctx.request       = modbus_rtu_master_request;
  mb_master_ctx_init(&master->ctx);

  master->stream.cb    = modbus_rtu_master_stream_callback;
  stream_init_with_fd(&master->stream, fd, master->rx_bounce_buf,  128, 512);

  evloop_timer_init(&master->t35, t35_timeout_handler, NULL);
  master->t35_val           = 5.0/1000;     // just 5ms max for every baud rate.
}

void
modbus_rtu_master_start(ModbusRTUMaster* master)
{
  stream_start(&master->stream);
}

void
modbus_rtu_master_stop(ModbusRTUMaster* master)
{
  stream_stop(&master->stream);
}

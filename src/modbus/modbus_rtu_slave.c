#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "common_def.h"
#include "modbus_slave.h"
#include "modbus_rtu_slave.h"
#include "modbus_crc.h"
#include "modbus_rtu_request_handler.h"
#include "modbus_funcs.h"

#include "hex_dump.h"
#include "trace.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// macros for convenience
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define INC_ERR_CNT(err_cnt)    \
  if(err_cnt < 0xffff)          \
  {                             \
    err_cnt++;                  \
  }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// private prototoypes
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void t35_timeout_handler(evloop_timer_t* te, void* unused);
static void mb_rtu_start_handling_rx_frame(ModbusRTUSlave* slave);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// buffer management
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void
mb_rtu_reset_data_buffer(ModbusRTUSlave* slave)
{
  slave->data_ndx   = 0;
  slave->tx_ndx     = 0;
}

/*
static inline void
mb_rtu_put_data(ModbusRTUSlave* slave, uint8_t b)
{
  slave->data_buffer[slave->data_ndx] = b;
  slave->data_ndx++;
}
*/

static inline uint16_t
mb_rtu_rx_len(ModbusRTUSlave* slave)
{
  return slave->data_ndx;
}

static inline uint8_t*
mb_rtu_buffer(ModbusRTUSlave* slave)
{
  return slave->data_buffer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// USART/RS486 specific
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void
mb_rtu_enable_rx(ModbusRTUSlave* slave)
{
  mb_rtu_reset_data_buffer(slave);

  // 
  // FIXME enable RX in case of half-duplex
  //
}

static inline void
mb_rtu_disable_rx(ModbusRTUSlave* slave)
{
  //
  // FIXME disable RX inc ase of half-duplex
  //
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// optional snooping
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__attribute__((weak))
void
modbus_rtu_frame_snoop(ModbusRTUSlave* slave,uint8_t addr, uint8_t* pdu, uint16_t len)
{
  // do nothing by default
}

__attribute__((weak))
void
modbus_rtu_init_snoop(ModbusRTUSlave* slave)
{
  // do nothing by default
}  


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MODBUS RX/TX in mainloop
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
t35_timeout_handler(evloop_timer_t* te, void* unused)
{
  ModbusRTUSlave*    slave = container_of(te, ModbusRTUSlave, t35);

  // for safely and to prevent fuck up, disable further rx here
  mb_rtu_disable_rx(slave);
  mb_rtu_start_handling_rx_frame(slave);
}

static void
mb_rtu_handle_rx_frame(ModbusRTUSlave* slave)
{
  uint8_t   addr,
            *pdu,
            ret;
  uint16_t  len,
            rsp_len,
            crc;
  ModbusSlaveCTX*    ctx = &slave->ctx;

  addr = mb_rtu_buffer(slave)[MB_SER_PDU_ADDR_OFF];
  len  = mb_rtu_rx_len(slave) - MB_SER_PDU_PDU_OFF - MB_SER_PDU_SIZE_CRC;
  pdu  = (uint8_t*)&mb_rtu_buffer(slave)[MB_SER_PDU_PDU_OFF];

  if(addr == slave->my_address || addr == MB_ADDRESS_BROADCAST)
  {
    ret = modbus_rtu_handler_request_rx(ctx, addr, len, pdu, &rsp_len);
    if(addr != MB_ADDRESS_BROADCAST)
    {
      ctx->my_frames++;
      if(ret != TRUE)
      {
        TRACE(MB_RTU_SLAVE, "modbus transaction error\n");
        ctx->req_fails++;
      }

      //
      // finish TX frame for RTU slave
      //
      slave->data_buffer[0] = addr;
      slave->data_ndx       = 1 + rsp_len;
      crc = modbus_calc_crc((uint8_t*)slave->data_buffer, slave->data_ndx);
  
      slave->data_buffer[slave->data_ndx++] = (uint8_t)(crc & 0xff);
      slave->data_buffer[slave->data_ndx++] = (uint8_t)(crc >> 8);
      
      TRACE_DUMP(MB_RTU_SLAVE, "MB RTU Slave TX", slave->data_buffer, slave->data_ndx);

      if(stream_write(&slave->stream, slave->data_buffer, slave->data_ndx) == FALSE)
      {
        TRACE(MB_RTU_SLAVE, "tx error\n");
      }
      else
      {
        ctx->tx_frames++;
      }
      mb_rtu_enable_rx(slave);
      return;
    }
  }
  else
  {
    modbus_rtu_frame_snoop(slave, addr, pdu, len);
  }
  mb_rtu_enable_rx(slave);
}

static void
mb_rtu_start_handling_rx_frame(ModbusRTUSlave* slave)
{
  ModbusSlaveCTX*    ctx = &slave->ctx;

  ctx->rx_frames++;

  TRACE_DUMP(MB_RTU_SLAVE, "MB RTU Slave RX", slave->data_buffer, slave->data_ndx);

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
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// MODBUS utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
init_modbus_context(ModbusRTUSlave* slave, uint8_t addr)
{
  ModbusSlaveCTX*    ctx = &slave->ctx;

  mb_slave_ctx_init(ctx);

  slave->my_address   = addr;
  slave->extension    = NULL;

  mb_rtu_reset_data_buffer(slave);

  evloop_timer_init(&slave->t35, t35_timeout_handler, NULL);

  // T35
  // at 9600    : 3ms +- alpha
  // above 9600 : 2ms should be enough
  //
  // so we can safely go with 5ms for t35.
  //
  slave->t35_val                  = 5.0/1000;     // just 5ms max for every baud rate.

  slave->rx_usart_overflow        = 0;
  slave->rx_usart_frame_error     = 0;
  slave->rx_usart_parity_error    = 0;
  slave->rx_short_frame           = 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// watcher callbacks
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
modbus_rtu_rx(ModbusRTUSlave* slave)
{
  ModbusSlaveCTX*    ctx = &slave->ctx;

  TRACE(SERIAL, "read %d bytes\n", slave->stream.rx_data_len);

  if(slave->data_ndx + slave->stream.rx_data_len >= MB_SER_RTU_PDU_SIZE_MAX)
  {
    TRACE(MB_RTU_SLAVE, , "read overflow %d\n");
    mb_rtu_reset_data_buffer(slave);
    INC_ERR_CNT(ctx->rx_buffer_overflow);
  }

  memcpy(&slave->data_buffer[slave->data_ndx], slave->stream.rx_buf, slave->stream.rx_data_len);
  slave->data_ndx += slave->stream.rx_data_len;

  evloop_timer_restart(&slave->t35, slave->t35_val, 0);
}

static void
modbus_rtu_stream_callback(stream_t* stream, stream_event_t evt)
{
  ModbusRTUSlave*    slave = container_of(stream, ModbusRTUSlave, stream);

  switch(evt)
  {
  case stream_event_rx:
    modbus_rtu_rx(slave);
    break;

  default:
    TRACE(MB_RTU_SLAVE, "stream_event : %d\n", evt);
    break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
modbus_rtu_slave_init(ModbusRTUSlave* slave, uint8_t device_addr, int fd)
{
  slave->stream.cb    = modbus_rtu_stream_callback;
  stream_init_with_fd(&slave->stream, fd, slave->rx_bounce_buf, 128, MB_SER_RTU_PDU_SIZE_MAX * 3);

  init_modbus_context(slave, device_addr);
  modbus_rtu_init_snoop(slave);
  mb_rtu_enable_rx(slave);
}

void
modbus_rtu_slave_start(ModbusRTUSlave* slave)
{
  stream_start(&slave->stream);
}

void
modbus_rtu_slave_stop(ModbusRTUSlave* slave)
{
  stream_stop(&slave->stream);
}

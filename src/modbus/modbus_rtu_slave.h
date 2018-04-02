#ifndef __MODBUS_RTU_SLAVE_DEF_H__
#define __MODBUS_RTU_SLAVE_DEF_H__

#include <stdint.h>
#include "evloop_timer.h"
#include "watcher.h"
#include "serial.h"
#include "modbus_slave.h"
#include "stream.h"

/*
 * Constants which defines the format of a modbus frame. The example is
 * shown for a Modbus RTU/ASCII frame. Note that the Modbus PDU is not
 * dependent on the underlying transport.
 *
 * <------------------------ MODBUS SERIAL LINE PDU (1) ------------------->
 *              <----------- MODBUS PDU (1') ---------------->
 *  +-----------+---------------+----------------------------+-------------+
 *  | Address   | Function Code | Data                       | CRC/LRC     |
 *  +-----------+---------------+----------------------------+-------------+
 *  |           |               |                                   |
 * (2)        (3/2')           (3')                                (4)
 *
 * (1)  ... MB_SER_PDU_SIZE_MAX = 256
 * (2)  ... MB_SER_PDU_ADDR_OFF = 0
 * (3)  ... MB_SER_PDU_PDU_OFF  = 1
 * (4)  ... MB_SER_PDU_SIZE_CRC = 2
 *
 * (1') ... MB_PDU_SIZE_MAX     = 253
 * (2') ... MB_PDU_FUNC_OFF     = 0
 * (3') ... MB_PDU_DATA_OFF     = 1
 */
#define MB_SER_RTU_PDU_SIZE_MIN         4
#define MB_SER_RTU_PDU_SIZE_MAX         256
#define MB_SER_PDU_SIZE_CRC             2
#define MB_SER_PDU_ADDR_OFF             0
#define MB_SER_PDU_PDU_OFF              1

typedef struct
{
  ModbusSlaveCTX        ctx;

  uint8_t               data_buffer[MB_SER_RTU_PDU_SIZE_MAX];
  uint8_t               rx_bounce_buf[128];
  uint16_t              data_ndx;
  uint16_t              tx_ndx;

  uint8_t               my_address;

  evloop_timer_t        t35;
  double                t35_val;

  // stats for debugging
  uint32_t              rx_usart_overflow;
  uint32_t              rx_usart_frame_error;
  uint32_t              rx_usart_parity_error;
  uint32_t              rx_short_frame;

  void*                 extension;                // an extension mechanism

  stream_t              stream;
} ModbusRTUSlave;

extern void modbus_rtu_slave_init(ModbusRTUSlave* slave, uint8_t device_addr, const char* port, SerialConfig* cfg);
extern void modbus_rtu_slave_start(ModbusRTUSlave* slave);
extern void modbus_rtu_slave_stop(ModbusRTUSlave* slave);

#endif /* !__MODBUS_RTU_SLAVE_DEF_H__ */

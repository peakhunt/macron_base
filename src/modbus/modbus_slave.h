#ifndef MODBUS_SLAVE_H_
#define MODBUS_SLAVE_H_

#include "modbus_common.h"

struct __modbus_slave_ctx;
typedef struct __modbus_slave_ctx ModbusSlaveCTX;

struct __modbus_slave_ctx
{
  uint32_t              rx_buffer_overflow;
  uint32_t              rx_crc_error;
  uint32_t              rx_frames;
  uint32_t              req_fails;
  uint32_t              my_frames;
  uint32_t              tx_frames;
  
  MBErrorCode           (*input_regs_cb)(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                            uint16_t usAddress, uint16_t usNRegs);
  MBErrorCode           (*holding_regs_cb)(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer,
                            uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode);
  MBErrorCode           (*coil_cb)(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress,
                            uint16_t usNRegs, MBRegisterMode eMode);
  MBErrorCode           (*discrete_cb)(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress,
                            uint16_t usNRegs);
};

static inline void
mb_slave_ctx_init(ModbusSlaveCTX* ctx)
{
  ctx->rx_buffer_overflow       = 0;
  ctx->rx_crc_error             = 0;
  ctx->rx_frames                = 0;
  ctx->req_fails                = 0;
  ctx->my_frames                = 0;
  ctx->tx_frames                = 0;
}

#endif /* MODBUS_SLAVE_H_ */

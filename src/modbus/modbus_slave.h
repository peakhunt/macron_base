#ifndef MODBUS_SLAVE_H_
#define MODBUS_SLAVE_H_

#include <stdio.h>
#include "modbus_common.h"
#include "list.h"
#include "cJSON.h"

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
  void*                 priv;
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

static inline void
mb_slave_ctx_get_stat(ModbusSlaveCTX* ctx, cJSON* jslave)
{
  cJSON_AddItemToObject(jslave, "rx_frames", cJSON_CreateNumber(ctx->rx_frames));
  cJSON_AddItemToObject(jslave, "tx_frames", cJSON_CreateNumber(ctx->tx_frames));
  cJSON_AddItemToObject(jslave, "my_frames", cJSON_CreateNumber(ctx->my_frames));
  cJSON_AddItemToObject(jslave, "req_fails", cJSON_CreateNumber(ctx->req_fails));
  cJSON_AddItemToObject(jslave, "rx_crc_error", cJSON_CreateNumber(ctx->rx_crc_error));
  cJSON_AddItemToObject(jslave, "rx_buffer_overflow", cJSON_CreateNumber(ctx->rx_buffer_overflow));
}

#endif /* MODBUS_SLAVE_H_ */

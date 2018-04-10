#include <assert.h>
#include <unistd.h>
#include <errno.h>

#include "common_def.h"
#include "modbus_master.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline void
mb_master_fill_io_status_req(ModbusMasterCTX* ctx,
    uint8_t function, uint16_t reg_addr, uint16_t nb_or_v)
{
  // transport specific header will be filled in later
  ctx->tx_ndx   = ctx->pdu_offset;

  ctx->tx_buf[ctx->tx_ndx++]  = function;
  ctx->tx_buf[ctx->tx_ndx++]  = reg_addr >> 8;
  ctx->tx_buf[ctx->tx_ndx++]  = reg_addr & 0xff;
  ctx->tx_buf[ctx->tx_ndx++]  = nb_or_v >> 8;
  ctx->tx_buf[ctx->tx_ndx++]  = nb_or_v & 0xff;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
mb_master_ctx_init(ModbusMasterCTX* ctx)
{
  // FIXME
}

void
mb_master_read_coils_req(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb)
{
  ctx->req_func     = MB_FUNC_READ_COILS;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = reg_addr;
  ctx->req_nb       = nb;

  mb_master_fill_io_status_req(ctx, MB_FUNC_READ_COILS, reg_addr, nb);
  ctx->request(ctx, slave);
}

void
mb_master_read_discrete_inputs(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb)
{
  ctx->req_func     = MB_FUNC_READ_DISCRETE_INPUTS;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = reg_addr;
  ctx->req_nb       = nb;

  mb_master_fill_io_status_req(ctx, MB_FUNC_READ_DISCRETE_INPUTS, reg_addr, nb);
  ctx->request(ctx, slave);
}

void
mb_master_read_holding_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb)
{
  ctx->req_func     = MB_FUNC_READ_HOLDING_REGISTER;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = reg_addr;
  ctx->req_nb       = nb;

  mb_master_fill_io_status_req(ctx, MB_FUNC_READ_HOLDING_REGISTER, reg_addr, nb);
  ctx->request(ctx, slave);
}

void
mb_master_read_input_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb)
{
  ctx->req_func     = MB_FUNC_READ_INPUT_REGISTER;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = reg_addr;
  ctx->req_nb       = nb;

  mb_master_fill_io_status_req(ctx, MB_FUNC_READ_INPUT_REGISTER, reg_addr, nb);
  ctx->request(ctx, slave);
}

void
mb_master_write_single_coil(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint8_t value)
{
  ctx->req_func     = MB_FUNC_WRITE_SINGLE_COIL;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = reg_addr;
  ctx->req_nb       = 1;
  ctx->req_value    = value ? 0xff00 : 0x0000;

  mb_master_fill_io_status_req(ctx, MB_FUNC_WRITE_SINGLE_COIL, reg_addr, value ? 0xff00 : 0x0000);
  ctx->request(ctx, slave);
}

void
mb_master_write_single_register(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t value)
{
  ctx->req_func     = MB_FUNC_WRITE_REGISTER;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = reg_addr;
  ctx->req_nb       = 1;
  ctx->req_value    = value;

  mb_master_fill_io_status_req(ctx, MB_FUNC_WRITE_REGISTER, reg_addr, value);
  ctx->request(ctx, slave);
}

void
mb_master_write_multiple_coils(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb,
    const uint8_t* values)
{
  uint8_t   byte_count,
            i;
  uint16_t  pos = 0;

  ctx->req_func     = MB_FUNC_WRITE_MULTIPLE_COILS;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = reg_addr;
  ctx->req_nb        = nb;

  mb_master_fill_io_status_req(ctx, MB_FUNC_WRITE_MULTIPLE_COILS, reg_addr, nb);

  byte_count = (nb / 8) + ((nb % 8) ? 1 : 0);

  ctx->tx_buf[ctx->tx_ndx++]    = byte_count;

  for (i = 0; i < byte_count; i++)
  {
    uint8_t bit;
    
    bit = 0x01;

    ctx->tx_buf[ctx->tx_ndx] = 0;
    
    for(bit = 0; bit < 8 && pos < nb; bit++, pos++)
    {
      if (values[pos])
        ctx->tx_buf[ctx->tx_ndx] |= (1 << bit);
    }
    ctx->tx_ndx++;
  }
  ctx->request(ctx, slave);
}

void
mb_master_write_multiple_registers(ModbusMasterCTX* ctx, uint8_t slave, uint16_t reg_addr, uint16_t nb,
    const uint16_t* values)
{
  uint8_t   byte_count;
  uint16_t  i;

  ctx->req_func     = MB_FUNC_WRITE_MULTIPLE_REGISTERS;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = reg_addr;
  ctx->req_nb       = nb;

  mb_master_fill_io_status_req(ctx, MB_FUNC_WRITE_MULTIPLE_REGISTERS, reg_addr, nb);

  byte_count = nb * 2;
  ctx->tx_buf[ctx->tx_ndx++]    = byte_count;

  for (i = 0; i < nb; i++)
  {
    ctx->tx_buf[ctx->tx_ndx++] = values[i] >> 8;
    ctx->tx_buf[ctx->tx_ndx++] = values[i] & 0xff;
  }

  ctx->request(ctx, slave);
}

void
mb_master_write_and_read_registers(ModbusMasterCTX* ctx, uint8_t slave,
    uint16_t write_addr, uint16_t nb_write, const uint16_t* values,
    uint16_t read_addr, uint16_t nb_read)
{
  uint8_t   byte_count;
  uint16_t  i;

  ctx->req_func     = MB_FUNC_READWRITE_MULTIPLE_REGISTERS;
  ctx->req_slave    = slave;
  ctx->req_reg_addr = read_addr;
  ctx->req_nb       = nb_read;

  mb_master_fill_io_status_req(ctx, MB_FUNC_READWRITE_MULTIPLE_REGISTERS, read_addr, nb_read);

  ctx->tx_buf[ctx->tx_ndx++] = write_addr >> 8;
  ctx->tx_buf[ctx->tx_ndx++] = write_addr & 0xff;
  ctx->tx_buf[ctx->tx_ndx++] = nb_write >> 8;
  ctx->tx_buf[ctx->tx_ndx++] = nb_write & 0xff;

  byte_count = nb_write * 2;
  ctx->tx_buf[ctx->tx_ndx++] = byte_count;

  for (i = 0; i < nb_write; i++)
  {
    ctx->tx_buf[ctx->tx_ndx++] = values[i] >> 8;
    ctx->tx_buf[ctx->tx_ndx++] = values[i] & 0xff;
  }

  ctx->request(ctx, slave);
}

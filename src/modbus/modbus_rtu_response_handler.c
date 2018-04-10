#include <stdio.h>
#include <stdint.h>
#include "common_def.h"
#include "trace.h"
#include "modbus_rtu_response_handler.h"
#include "modbus_funcs.h"

static bool
modbus_rtu_validate_response(ModbusMasterCTX* ctx, uint8_t slave, uint8_t* pdu, int pdu_len)
{
  uint16_t      nb,
                addr,
                value;

  if(ctx->req_func != pdu[0])
  {
    return FALSE;
  }

  switch(ctx->req_func)
  {
  case MB_FUNC_READ_COILS:
  case MB_FUNC_READ_DISCRETE_INPUTS:
    if(pdu_len != (2 + pdu[1]))
    {
      TRACE(MB_MASTER, "pdu_len mismatch, %d,%d,%d \n", ctx->req_func, pdu_len, (2 + pdu[1]));
      return FALSE;
    }

    nb = pdu[1] * 8;
    if(nb < ctx->req_nb)
    {
      TRACE(MB_MASTER, "nb mismatch, %d,%d,%d \n", ctx->req_func, ctx->req_nb, nb);
      return FALSE;
    }
    break;

  case MB_FUNC_READ_HOLDING_REGISTER:
  case MB_FUNC_READ_INPUT_REGISTER:
    if(pdu_len != (2 + pdu[1]))
    {
      TRACE(MB_MASTER, "pdu_len mismatch, %d,%d,%d \n", ctx->req_func, pdu_len, (2 + pdu[1]));
      return FALSE;
    }

    nb = pdu[1] / 2;

    if(ctx->req_nb != nb)
    {
      TRACE(MB_MASTER, "nb mismatch, %d,%d,%d \n", ctx->req_func, ctx->req_nb, nb);
      return FALSE;
    }
    break;

  case MB_FUNC_WRITE_SINGLE_COIL:
  case MB_FUNC_WRITE_REGISTER:
    /* normal response is an echo of the request */
    /* FIXME request value check */
    if(pdu_len != 5)
    {
      TRACE(MB_MASTER, "pdu_len mismatch, %d,%d,%d \n", ctx->req_func, pdu_len, 5);
      return FALSE;
    }

    value = ((pdu[3] << 8) | pdu[4]);
    if(value != ctx->req_value)
    {
      TRACE(MB_MASTER, "value mismatch, %d,%d,%d \n", ctx->req_func, ctx->req_value, value);
      return FALSE;
    }

    addr = ((pdu[1] << 8) | pdu[2]);
    if(addr != ctx->req_reg_addr)
    {
      TRACE(MB_MASTER, "addr mismatch, %d,%d,%d \n", ctx->req_func, ctx->req_reg_addr, addr);
      return FALSE;
    }
    break;

  case MB_FUNC_WRITE_MULTIPLE_COILS:
  case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
    if(pdu_len != 5)
    {
      TRACE(MB_MASTER, "pdu_len mismatch, %d,%d,%d \n", ctx->req_func, pdu_len, 5);
      return FALSE;
    }

    addr  = ((pdu[1] << 8) | pdu[2]);
    nb    = ((pdu[3] << 8) | pdu[4]);

    if(addr != ctx->req_reg_addr || nb    != ctx->req_nb)
    {
      TRACE(MB_MASTER, "addr or nb mismatch, %d,%d,%d,%d,%d \n",
          addr, ctx->req_reg_addr,
          ctx->req_func, ctx->req_nb, nb);
      return FALSE;
    }
    break;

  case MB_FUNC_READWRITE_MULTIPLE_REGISTERS:
    if(pdu_len != (2 + pdu[1]))
    {
      TRACE(MB_MASTER, "pdu_len mismatch, %d,%d,%d \n", ctx->req_func, pdu_len, (2 + pdu[1]));
      return FALSE;
    }

    nb = pdu[1] / 2;
    if(ctx->req_nb != nb)
    {
      TRACE(MB_MASTER, "nb mismatch, %d,%d,%d \n", ctx->req_func, ctx->req_nb, nb);
      return FALSE;
    }
    break;

  default:
    return FALSE;
  }

  return TRUE;
}

void
modbus_rtu_handler_response_rx(ModbusMasterCTX* ctx, uint8_t slave, uint8_t* pdu, int pdu_len)
{
  if(modbus_rtu_validate_response(ctx, slave, pdu, pdu_len) != TRUE)
  {
    TRACE(MB_MASTER, "modbus rtu validate failed \n");
    return;
  }

  TRACE(MB_MASTER, "modbus rtu validate pass !!!!!  \n");

  switch(ctx->req_func)
  {
  case MB_FUNC_READ_COILS:
    ctx->coil_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, &pdu[2], MB_REG_READ);
    break;

  case MB_FUNC_READ_DISCRETE_INPUTS:
    ctx->discrete_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, &pdu[2]);
    break;

  case MB_FUNC_READ_HOLDING_REGISTER:
    ctx->holding_regs_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, &pdu[2], MB_REG_READ);
    break;

  case MB_FUNC_READ_INPUT_REGISTER:
    ctx->input_regs_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, &pdu[2]);
    break;

  case MB_FUNC_WRITE_SINGLE_COIL:
    ctx->coil_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, &pdu[3], MB_REG_WRITE);
    break;

  case MB_FUNC_WRITE_REGISTER:
    ctx->holding_regs_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, &pdu[3], MB_REG_WRITE); 
    break;

  case MB_FUNC_WRITE_MULTIPLE_COILS:
    ctx->coil_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, NULL, MB_REG_WRITE);
    break;

  case MB_FUNC_WRITE_MULTIPLE_REGISTERS:
    ctx->holding_regs_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, NULL, MB_REG_WRITE);
    break;

  case MB_FUNC_READWRITE_MULTIPLE_REGISTERS:
    // this is interesting XXX for now only read callback
    //
    // FIXME confirmation for WRITE
    //
    ctx->holding_regs_cb(ctx, slave, ctx->req_reg_addr, ctx->req_nb, NULL, MB_REG_READ);
    break;

  default:
    // impossible to reach here
    break;
  }
}

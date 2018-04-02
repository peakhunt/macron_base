#include <stdint.h>
#include "modbus_slave.h"
#include "modbus_util.h"
#include "modbus_funcs.h"

#define MB_PDU_FUNC_READ_ADDR_OFF           ( MB_PDU_DATA_OFF )
#define MB_PDU_FUNC_READ_REGCNT_OFF         ( MB_PDU_DATA_OFF + 2 )
#define MB_PDU_FUNC_READ_SIZE               ( 4 )
#define MB_PDU_FUNC_READ_REGCNT_MAX         ( 0x007D )

#define MB_PDU_FUNC_READ_RSP_BYTECNT_OFF    ( MB_PDU_DATA_OFF )

MBException
modbus_func_handler_read_input_register(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen )
{
  uint16_t          usRegAddress;
  uint16_t          usRegCount;
  uint8_t          *pucFrameCur;

  MBException    eStatus = MB_EX_NONE;
  MBErrorCode    eRegStatus;

  if( *usLen == ( MB_PDU_FUNC_READ_SIZE + MB_PDU_SIZE_MIN ) )
  {
    usRegAddress = ( uint16_t )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF] << 8 );
    usRegAddress |= ( uint16_t )( pucFrame[MB_PDU_FUNC_READ_ADDR_OFF + 1] );
#ifdef MODBUS_USE_REGISTER_NUMBER
    usRegAddress++;
#endif

    usRegCount = ( uint16_t )( pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF] << 8 );
    usRegCount |= ( uint16_t )( pucFrame[MB_PDU_FUNC_READ_REGCNT_OFF + 1] );

    /* Check if the number of registers to read is valid. If not
     * return Modbus illegal data value exception.
     */
    if( ( usRegCount >= 1 )
        && ( usRegCount < MB_PDU_FUNC_READ_REGCNT_MAX ) )
    {
      /* Set the current PDU data pointer to the beginning. */
      pucFrameCur = &pucFrame[MB_PDU_FUNC_OFF];
      *usLen = MB_PDU_FUNC_OFF;

      /* First byte contains the function code. */
      *pucFrameCur++ = MB_FUNC_READ_INPUT_REGISTER;
      *usLen += 1;

      /* Second byte in the response contain the number of bytes. */
      *pucFrameCur++ = ( uint8_t )( usRegCount * 2 );
      *usLen += 1;

      eRegStatus = ctx->input_regs_cb(ctx, addr, pucFrameCur, usRegAddress, usRegCount );

      /* If an error occured convert it into a Modbus exception. */
      if( eRegStatus != MB_ENOERR )
      {
        eStatus = prveMBError2Exception( eRegStatus );
      }
      else
      {
        *usLen += usRegCount * 2;
      }
    }
    else
    {
      eStatus = MB_EX_ILLEGAL_DATA_VALUE;
    }
  }
  else
  {
    /* Can't be a valid read input register request because the length
     * is incorrect. */
    eStatus = MB_EX_ILLEGAL_DATA_VALUE;
  }
  return eStatus;
}

#ifndef __MODBUS_COMMON_DEF_H__
#define __MODBUS_COMMON_DEF_H__

#include <stdint.h>

#define MB_PDU_SIZE_MAX                 253
#define MB_PDU_SIZE_MIN                 1
#define MB_PDU_FUNC_OFF                 0
#define MB_PDU_DATA_OFF                 1

#define MB_ADDRESS_BROADCAST            0
#define MB_ADDRESS_MIN                  1
#define MB_ADDRESS_MAX                  247

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// a bunch of definitions for standard MODBUS RTU
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MB_FUNC_NONE                          (  0 )
#define MB_FUNC_READ_COILS                    (  1 )
#define MB_FUNC_READ_DISCRETE_INPUTS          (  2 )
#define MB_FUNC_WRITE_SINGLE_COIL             (  5 )
#define MB_FUNC_WRITE_MULTIPLE_COILS          ( 15 )
#define MB_FUNC_READ_HOLDING_REGISTER         (  3 )
#define MB_FUNC_READ_INPUT_REGISTER           (  4 )
#define MB_FUNC_WRITE_REGISTER                (  6 )
#define MB_FUNC_WRITE_MULTIPLE_REGISTERS      ( 16 )
#define MB_FUNC_READWRITE_MULTIPLE_REGISTERS  ( 23 )
#define MB_FUNC_DIAG_READ_EXCEPTION           (  7 )
#define MB_FUNC_DIAG_DIAGNOSTIC               (  8 )
#define MB_FUNC_DIAG_GET_COM_EVENT_CNT        ( 11 )
#define MB_FUNC_DIAG_GET_COM_EVENT_LOG        ( 12 )
#define MB_FUNC_OTHER_REPORT_SLAVEID          ( 17 )
#define MB_FUNC_ERROR                         ( 128)
 
typedef enum
{
  MB_EX_NONE                    = 0x00,
  MB_EX_ILLEGAL_FUNCTION        = 0x01,
  MB_EX_ILLEGAL_DATA_ADDRESS    = 0x02,
  MB_EX_ILLEGAL_DATA_VALUE      = 0x03,
  MB_EX_SLAVE_DEVICE_FAILURE    = 0x04,
  MB_EX_ACKNOWLEDGE             = 0x05,
  MB_EX_SLAVE_BUSY              = 0x06,
  MB_EX_MEMORY_PARITY_ERROR     = 0x08,
  MB_EX_GATEWAY_PATH_FAILED     = 0x0A,
  MB_EX_GATEWAY_TGT_FAILED      = 0x0B
} MBException;

typedef enum
{
  MB_ENOERR,                  /*!< no error. */
  MB_ENOREG,                  /*!< illegal register address. */
  MB_EINVAL,                  /*!< illegal argument. */
  MB_EPORTERR,                /*!< porting layer error. */
  MB_ENORES,                  /*!< insufficient resources. */
  MB_EIO,                     /*!< I/O error. */
  MB_EILLSTATE,               /*!< protocol stack in illegal state. */
  MB_ETIMEDOUT                /*!< timeout error occurred. */
} MBErrorCode;

typedef enum
{
  MB_REG_READ,                /*!< Read register values and pass to protocol stack. */
  MB_REG_WRITE                /*!< Update register values. */
} MBRegisterMode;

#endif /* !__MODBUS_COMMON_DEF_H__ */

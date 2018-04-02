#ifndef MODBUS_FUNCS_H_
#define MODBUS_FUNCS_H_

#include <stdint.h>
#include "modbus_slave.h"

extern MBException modbus_func_handler_read_coils(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen);
extern MBException modbus_func_handler_write_coil(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen);
extern MBException modbus_func_handler_write_multiple_coils(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen);

extern MBException modbus_func_handler_write_holding_register(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen);
extern MBException modbus_func_handler_write_multiple_holding_registers(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen);
extern MBException modbus_func_handler_read_holding_register(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen);
extern MBException modbus_func_handler_read_write_multiple_holding_registers(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen);

extern MBException modbus_func_handler_read_input_register(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t* pucFrame, uint16_t * usLen);

extern MBException modbus_func_handler_read_discrete_inputs(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucFrame, uint16_t * usLen);


////////////////////////////////////////////////////////////////////////////////
//
// The following callbacks are expected to be implemented by the user
//
////////////////////////////////////////////////////////////////////////////////
extern MBErrorCode modbus_user_coil_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNCoils, MBRegisterMode eMode);
extern MBErrorCode modbus_user_holding_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs, MBRegisterMode eMode);
extern MBErrorCode modbus_user_input_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNRegs);
extern MBErrorCode modbus_user_discrete_cb(ModbusSlaveCTX* ctx, uint8_t addr, uint8_t * pucRegBuffer, uint16_t usAddress, uint16_t usNDiscrete);

#endif /* MODBUS_FUNCS_H_ */

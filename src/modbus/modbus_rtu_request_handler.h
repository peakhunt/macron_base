#ifndef MODBUS_RTU_REQUEST_HANDLER_H_
#define MODBUS_RTU_REQUEST_HANDLER_H_

#include "modbus_slave.h"

extern uint8_t modbus_rtu_handler_request_rx(ModbusSlaveCTX* ctx, uint8_t addr, uint16_t len, uint8_t* pdu, uint16_t* rsp_len);

#endif /* MODBUS_RTU_REQUEST_HANDLER_H_ */

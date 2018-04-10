#ifndef MODBUS_RTU_RESPONSE_HANDLER_H__
#define MODBUS_RTU_RESPONSE_HANDLER_H__

#include "modbus_master.h"

extern void modbus_rtu_handler_response_rx(ModbusMasterCTX* ctx, uint8_t slave, uint8_t* pdu, int pdu_len);
#endif /* !MODBUS_RTU_RESPONSE_HANDLER_H__ */

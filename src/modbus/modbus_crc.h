#ifndef MODBUS_CRC_H_
#define MODBUS_CRC_H_

#include <stdint.h>

extern uint16_t modbus_calc_crc(uint8_t* buffer, uint16_t len);

#endif /* MODBUS_CRC_H_ */

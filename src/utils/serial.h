#ifndef __SERIAL_DEF_H__
#define __SERIAL_DEF_H__

#include <termios.h>

typedef enum
{
  SerialParity_None,
  SerialParity_Even,
  SerialParity_Odd,
} SerialParity;

typedef struct
{
  int           baud;         // baud rate : 9600, 19200, ...
  char          data_bit;     // 7 or 8
  char          stop_bit;     // 1 or 2
  SerialParity  parity;
} SerialConfig;

extern int serial_init(const char* path, const SerialConfig* config);

#endif /* !__SERIAL_DEF_H__ */

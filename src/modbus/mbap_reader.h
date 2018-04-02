#ifndef __MBAP_READER_DEF_H__
#define __MBAP_READER_DEF_H__

#include "modbus_tcp_slave.h"

struct __mbap_reader;
typedef struct __mbap_reader mbap_reader_t;

typedef void (*mbap_reader_cb)(mbap_reader_t* mbap);

typedef enum
{
  mbap_reader_state_header,
  mbap_reader_state_pdu,
} mbap_reader_state_t;

struct __mbap_reader
{
  uint8_t               frame[MB_TCP_PDU_SIZE_MAX];
  mbap_reader_state_t   state;

  uint16_t              tid;
  uint16_t              pid;
  uint16_t              length;
  uint8_t               uid;

  int                   header_remaining;
  int                   data_remaining;
  int                   ndx;

  mbap_reader_cb        cb;
};

extern void mbap_reader_init(mbap_reader_t* mbap, mbap_reader_cb cb);
extern void mbap_reader_reset(mbap_reader_t* mbap);
extern void mbap_reader_feed(mbap_reader_t* mbap, uint8_t* buf, int len);

#endif /* !__MBAP_READER_DEF_H__ */

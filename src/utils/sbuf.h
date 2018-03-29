#ifndef __SBUF_DEF_H__
#define __SBUF_DEF_H__

#include "common_def.h"

typedef struct
{
  uint8_t*      buf; 
  uint32_t      len;
  uint32_t      data_len;
  uint32_t      write_ndx;
  uint32_t      read_ndx;
} sbuf_t;

extern void sbuf_init(sbuf_t* sbuf, uint8_t* buf, uint32_t len);

extern bool sbuf_write_u8(sbuf_t* sbuf, uint8_t data);
extern bool sbuf_write_s8(sbuf_t* sbuf, int8_t data);
extern bool sbuf_write_u16(sbuf_t* sbuf, uint16_t data);
extern bool sbuf_write_s16(sbuf_t* sbuf, int16_t data);
extern bool sbuf_write_u32(sbuf_t* sbuf, uint32_t data);
extern bool sbuf_write_s32(sbuf_t* sbuf, int32_t data);

extern bool sbuf_read_u8(sbuf_t* sbuf, uint8_t* data);
extern bool sbuf_read_s8(sbuf_t* sbuf, int8_t* data);
extern bool sbuf_read_u16(sbuf_t* sbuf, uint16_t* data);
extern bool sbuf_read_s16(sbuf_t* sbuf, int16_t* data);
extern bool sbuf_read_u32(sbuf_t* sbuf, uint32_t* data);
extern bool sbuf_read_s32(sbuf_t* sbuf, int32_t* data);

extern bool sbuf_read(sbuf_t* sbuf, uint8_t* buf, int len);
extern bool sbuf_write(sbuf_t* sbuf, uint8_t* buf, int len);

#endif /* !__SBUF_DEF_H__ */

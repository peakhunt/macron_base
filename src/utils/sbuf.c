#include <stdio.h>
#include <arpa/inet.h>
#include "sbuf.h"

static inline bool
sbuf_is_write_possible(sbuf_t* sbuf, uint32_t len)
{
  if((sbuf->write_ndx + len) < sbuf->len)
  {
    return TRUE;
  }
  return FALSE;
}

static inline bool
sbuf_is_read_possible(sbuf_t* sbuf, uint32_t len)
{
  if((sbuf->read_ndx + len) < sbuf->data_len)
  {
    return TRUE;
  }
  return FALSE;
}

void
sbuf_init(sbuf_t* sbuf, uint8_t* buf, uint32_t len)
{
  sbuf->buf         = buf;
  sbuf->len         = len;
  sbuf->write_ndx   = 0;
  sbuf->read_ndx    = 0;
  sbuf->data_len     = 0;
}

bool
sbuf_read(sbuf_t* sbuf, uint8_t* buf, int len)
{
  int   i;

  if(!sbuf_is_read_possible(sbuf, len))
  {
    return FALSE;
  }

  for(i = 0; i < len; i++)
  {
    buf[i] = sbuf->buf[sbuf->read_ndx++];
  }

  return TRUE;
}

bool
sbuf_write(sbuf_t* sbuf, uint8_t* buf, int len)
{
  int   i;

  if(!sbuf_is_write_possible(sbuf, len))
  {
    return FALSE;
  }

  for(i = 0; i < len; i++)
  {
    sbuf->buf[sbuf->write_ndx++] = buf[i];
  }
  return TRUE;
}

bool
sbuf_write_u8(sbuf_t* sbuf, uint8_t data)
{
  return sbuf_write(sbuf, &data, sizeof(uint8_t));
}

bool
sbuf_write_s8(sbuf_t* sbuf, int8_t data)
{
  return sbuf_write(sbuf, (uint8_t*)&data, sizeof(int8_t));
}

bool
sbuf_write_u16(sbuf_t* sbuf, uint16_t data)
{
  uint16_t    d = htons(data);

  return sbuf_write(sbuf, (uint8_t*)&d, sizeof(uint16_t));
}

bool
sbuf_write_s16(sbuf_t* sbuf, int16_t data)
{
  uint16_t    d = htons(data);

  return sbuf_write(sbuf, (uint8_t*)&d, sizeof(int16_t));
}

bool
sbuf_write_u32(sbuf_t* sbuf, uint32_t data)
{
  uint32_t    d = htonl(data);

  return sbuf_write(sbuf, (uint8_t*)&d, sizeof(uint32_t));
}

bool
sbuf_write_s32(sbuf_t* sbuf, int32_t data)
{
  uint32_t    d = htonl(data);

  return sbuf_write(sbuf, (uint8_t*)&d, sizeof(int32_t));
}

bool
sbuf_read_u8(sbuf_t* sbuf, uint8_t* data)
{
  return sbuf_read(sbuf, data, sizeof(uint8_t));
}

bool
sbuf_read_s8(sbuf_t* sbuf, int8_t* data)
{
  return sbuf_read(sbuf, (uint8_t*)data, sizeof(int8_t));
}

bool
sbuf_read_u16(sbuf_t* sbuf, uint16_t* data)
{
  uint16_t    d;

  if(sbuf_read(sbuf, (uint8_t*)&d, sizeof(uint16_t)))
  {
    *data = ntohs(d);
    return TRUE;
  }
  return FALSE;
}

bool
sbuf_read_s16(sbuf_t* sbuf, int16_t* data)
{
  uint16_t    d;

  if(sbuf_read(sbuf, (uint8_t*)&d, sizeof(int16_t)))
  {
    *data = (int16_t)ntohs(d);
    return TRUE;
  }
  return FALSE;
}

bool
sbuf_read_u32(sbuf_t* sbuf, uint32_t* data)
{
  uint32_t    d;

  if(sbuf_read(sbuf, (uint8_t*)&d, sizeof(uint32_t)))
  {
    *data = ntohl(d);
    return TRUE;
  }
  return FALSE;
}

bool
sbuf_read_s32(sbuf_t* sbuf, int32_t* data)
{
  int32_t     d;

  if(sbuf_read(sbuf, (uint8_t*)&d, sizeof(int32_t)))
  {
    *data = (int32_t)ntohl(d);
    return TRUE;
  }
  return FALSE;
}

#ifndef __BIT_UTIL_DEF_H__
#define __BIT_UTIL_DEF_H__

#include <stdint.h>

static inline void
u16_mask(int start, int end, uint16_t* mask, uint8_t* shift)
{
  uint16_t    ret_mask = 0;

  *shift = start;

  for(int i = start; i <= end; i++)
  {
    ret_mask |= (1 << i);
  }

  *mask = ret_mask;
}

static inline uint16_t
u16_filter(uint16_t mask, uint8_t shift, uint16_t v)
{
  uint16_t    ret;

  ret = (v & mask) >> (shift);
  return ret;
}

#endif /* !__BIT_UTIL_DEF_H__ */

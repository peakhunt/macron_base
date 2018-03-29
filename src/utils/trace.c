#include "trace.h"

#define NDX_AND_BITNUM(comp)        \
  int       ndx;                    \
  int       bit_num;                \
  ndx     = comp / 32;              \
  bit_num = comp % 32;

static uint32_t    _trace_mask[NUM_TRACE_ARRAY];

void
trace_init(uint32_t* inits, int num_elem)
{
  for(int i = 0; i < num_elem; i++)
  {
    trace_on(inits[i]);
  }
}

void
trace_on(uint32_t comp)
{
  NDX_AND_BITNUM(comp);

  _trace_mask[ndx] |= (1 << bit_num);
}

void
trace_off(uint32_t comp)
{
  NDX_AND_BITNUM(comp);

  _trace_mask[ndx] &= ~(1 << bit_num);
}

bool
trace_is_on(uint32_t comp)
{
  NDX_AND_BITNUM(comp);

  return (_trace_mask[ndx] & (1 << bit_num)) == 0 ? FALSE : TRUE;
}

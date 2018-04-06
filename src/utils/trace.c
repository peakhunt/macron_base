#include <string.h>
#include "trace.h"

#define NDX_AND_BITNUM(comp)        \
  int       ndx;                    \
  int       bit_num;                \
  ndx     = comp / 32;              \
  bit_num = comp % 32;

const char* _trace_comp_strs[] = 
{
  "MAIN",
  "TEST",
  "TASK",
  "SOCK_ERR",
  "SERIAL",
  "STREAM",
  "CLI_TELNET",
  "CLI_SERIAL",
  "CLI",
  "MB_TCP_SLAVE",
  "MBAP",
  "MB_RTU_SLAVE",
  "MB_RTU_MASTER",
  "MB_TCP_MASTER",
  "MB_MASTER",
  CORE_TRACE_STRS
  APP_TRACE_STRS
};

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

int
trace_get_str_ndx(const char* name)
{
  for(int i = 0; i < NARRAY(_trace_comp_strs); i++)
  {
    if(strcmp(name, _trace_comp_strs[i]) == 0)
    {
      return i;
    }
  }
  return -1;
}

const char**
trace_get_strs(int* num_elem)
{
  *num_elem = NARRAY(_trace_comp_strs);
  return _trace_comp_strs;
}

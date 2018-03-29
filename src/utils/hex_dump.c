#include <stdio.h>
#include "common_def.h"
#include "trace.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// debug packet dump
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
hex_dump_buffer(const char* title, const char* head, uint8_t* buf, int len)
{
  char  buffer[128];
  int   i,
        j,
        row_len,
        buf_ndx;

  debug_log("===== %s, len: %d =====\n", title, len);
  for(i = 0; i < len; i += 8)
  {
    row_len = (len - i);
    row_len = row_len >= 8 ? 8 : row_len;

    buf_ndx = 0;

    for(j = 0; j < row_len; j++)
    {
      buf_ndx += sprintf(&buffer[buf_ndx], "%02x ", buf[i + j]);
    }

    debug_log("%s %04d: %s\n", head, i / 8, buffer);
  }
}


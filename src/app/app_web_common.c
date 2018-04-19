#include <string.h>
#include "app_web_common.h"

const char*
app_web_to_c_str(struct mg_str* ms)
{
  static char str[1024 + 1];
  int   len;
  
  len = ms->len <= 1024 ? ms->len : 1024;
  strncpy(str, ms->p, len);
  str[len] = '\0';
  
  return str;
}

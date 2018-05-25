#include <string.h>
#include "mongoose_util.h"

const char*
mg_util_to_c_str(struct mg_str* ms)
{
  static char str[1024 + 1];
  int   len;
  
  len = ms->len <= 1024 ? ms->len : 1024;
  strncpy(str, ms->p, len);
  str[len] = '\0';
  
  return str;
}

char*
mg_util_to_c_str_alloc(struct mg_str* ms)
{
  char* str;
  
  str = malloc(ms->len + 1);

  // FIXME check if malloc fails

  strncpy(str, ms->p, ms->len);
  str[ms->len] = '\0';
  
  return str;
}

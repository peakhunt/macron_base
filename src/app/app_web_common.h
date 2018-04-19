#ifndef __APP_WEB_COMMON_DEF_H__
#define __APP_WEB_COMMON_DEF_H__

#include <string.h>
#include "common_def.h"
#include "mongoose.h"

extern const char* app_web_to_c_str(struct mg_str* ms);

static inline bool
app_web_has_prefix(const struct mg_str* uri, const struct mg_str* prefix)
{
  return uri->len > prefix->len &&
         memcmp(uri->p, prefix->p, prefix->len) == 0;
}

static inline bool
app_web_is_equal(const struct mg_str* s1, const struct mg_str* s2)
{
  return s1->len == s2->len && memcmp(s1->p, s2->p, s2->len) == 0;
}

static inline int
app_web_get_int(const struct mg_str* substr)
{
  char      c;
  struct mg_str   ms = *substr;

  c = ms.p[ms.len-1];

  if(c == '/' || c == '?')
  {
    ms.len -= 1;
  }
  return atoi(app_web_to_c_str(&ms));
}

#endif /* !__APP_WEB_COMMON_DEF_H__ */

#ifndef __JSON_UTIL_DEF_H__
#define __JSON_UTIL_DEF_H__

#include "common_def.h"
#include "cJSON.h"

typedef struct
{
  const char*   name;
  int           cjson_type;
} json_util_field_t;

extern bool json_util_simple_validate_message(const cJSON* msg, const json_util_field_t* items, int num_items);

#endif /* !__JSON_UTIL_DEF_H__ */

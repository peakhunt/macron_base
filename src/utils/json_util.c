#include "json_util.h"

bool
json_util_simple_validate_message(const cJSON* msg, const json_util_field_t* items, int num_items)
{
  cJSON*    node;

  for(int i = 0; i < num_items; i++)
  {
    node = cJSON_GetObjectItem(msg, items[i].name);
    if(node == NULL)
    {
      return FALSE;
    }

    if((node->type & items[i].cjson_type) == 0)
    {
      return FALSE;
    }
  }
  return TRUE;
}

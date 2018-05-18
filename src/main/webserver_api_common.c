#include "webserver_api_common.h"

struct mg_str   _op_get   = MG_MK_STR("GET"),
                _op_put   = MG_MK_STR("PUT"),
                _op_post  = MG_MK_STR("POST");

cJSON*
webapi_parse_json_body(struct mg_str* str, struct mg_connection* nc, struct http_message* hm)
{
  cJSON*              json;
  char*               body;

  body = mg_util_to_c_str_alloc(str);
  if(body == NULL)
  {
    webapi_server_error(nc, hm);
    return NULL;
  }

  json = cJSON_Parse(body);
  free(body);

  if(json == NULL)
  {
    webapi_bad_request(nc, hm);
    return NULL;
  }

  return json;
}

#ifndef __WEBSERVER_API_COMMON_DEF_H__
#define __WEBSERVER_API_COMMON_DEF_H__

#include "cJSON.h"
#include "mongoose.h"
#include "mongoose_util.h"

extern struct mg_str   _op_get;
extern struct mg_str   _op_put;
extern struct mg_str   _op_post;

static inline void
webapi_not_found(struct mg_connection* nc, struct http_message* hm)
{
  mg_printf(nc, "%s",
      "HTTP/1.1 404 Not Found\r\n"
      "Content-Length: 0\r\n\r\n");
}

static inline void
webapi_bad_request(struct mg_connection* nc, struct http_message* hm)
{
  mg_printf(nc, "%s",
      "HTTP/1.1 400 Bad Request\r\n"
      "Content-Length: 0\r\n\r\n");
}

static inline void
webapi_server_error(struct mg_connection* nc, struct http_message* hm)
{
  mg_printf(nc, "%s",
      "HTTP/1.1 500 Server Error\r\n"
      "Content-Length: 0\r\n\r\n");
}

static inline void
webapi_server_error_other(struct mg_connection* nc, struct http_message* hm)
{
  mg_printf(nc, "%s",
      "HTTP/1.1 503 Server Error\r\n"
      "Content-Length: 0\r\n\r\n");
}

static inline void
webapi_server_json_response_ok(struct mg_connection* nc, const char* json, int len)
{
  mg_printf(nc,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/json\r\n"
      "Content-Length: %d\r\n\r\n%s",
      len, json);
}

static inline void
webapi_server_response_ok(struct mg_connection* nc)
{
  mg_printf(nc,
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 0\r\n\r\n");
}

extern cJSON* webapi_parse_json_body(struct mg_str* str, struct mg_connection* nc, struct http_message* hm);

#endif /* !__WEBSERVER_API_COMMON_DEF_H__ */

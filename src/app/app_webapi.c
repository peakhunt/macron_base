#include "common_def.h"
#include "channel_manager.h"
#include "alarm_manager.h"
#include "trace.h"
#include "app_webapi.h"
#include "app_web_common.h"

struct __api_cmd_handler_t;
typedef struct __api_cmd_handler_t api_cmd_handler_t;

struct __api_cmd_handler_t
{
  struct mg_str       prefix;
  api_cmd_handler_t*  sub;
  int                 n_sub;
  void (*handler)(struct mg_connection* nc, struct http_message* hm, struct mg_str* prev);
};

/*
   API List

   GET /api/v1/channel/status/<channel_num>
       {
          "eng_value": xxx or true or false
          "raw_val": xxx
       }

   GET /api/v1/alarm/status/<alarm_num>
      {
        "state":    "inactive" or
                    "pending" or
                    "inactive_pending" or
                    "active"
      }
*/
///////////////////////////////////////////////////////////////////////////////
//
// common error handler
//
///////////////////////////////////////////////////////////////////////////////
static inline void
app_webapi_not_found(struct mg_connection* nc, struct http_message* hm)
{
  mg_printf(nc, "%s",
      "HTTP/1.1 404 Not Found\r\n"
      "Content-Length: 0\r\n\r\n");
}

static inline void
app_webapi_server_error(struct mg_connection* nc, struct http_message* hm)
{
  mg_printf(nc, "%s",
      "HTTP/1.1 500 Server Error\r\n"
      "Content-Length: 0\r\n\r\n");
}


///////////////////////////////////////////////////////////////////////////////
//
// common api handler
//
///////////////////////////////////////////////////////////////////////////////
static void
execute_api_handler(api_cmd_handler_t* table, int num_entries,
    struct mg_connection* nc, struct http_message* hm, struct mg_str* current)
{
  struct mg_str         next;

  for(int i = 0; i < num_entries; i++)
  {
    if(app_web_has_prefix(current, &table[i].prefix) == true)
    {
      next.p    = current->p    + table[i].prefix.len;
      next.len  = current->len  - table[i].prefix.len;

      if(table[i].handler != NULL)
      {
        table[i].handler(nc, hm, &next);
      }
      else
      {
        execute_api_handler(table[i].sub, table[i].n_sub, nc, hm, &next);
      }
      return;
    }
  }

  TRACE(APP_WEB, "can't find target handler: %s\n", app_web_to_c_str(current));
  app_webapi_not_found(nc, hm);
}

///////////////////////////////////////////////////////////////////////////////
//
// API command handlers
//
///////////////////////////////////////////////////////////////////////////////
static void
app_webapi_channel_status(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  uint32_t            chnl_num;
  channel_status_t    status;
  char                data[128];

  chnl_num = (uint32_t)app_web_get_int(subcmd);

  TRACE(APP_WEB, "channel status request for %d\n", chnl_num);

  if(channel_manager_get_channel_stat(chnl_num, &status) == -1)
  {
    app_webapi_not_found(nc, hm);
    return;
  }

  if(status.chnl_type == channel_type_digital)
  {
    sprintf(data, "{ \"eng_value\": %.2f, \"raw_val\": %d }",
        status.eng_val.f,
        status.raw_val);
  }
  else
  {
    sprintf(data, "{ \"eng_value\": %s, \"raw_val\": %d }",
        status.eng_val.b ? "true" : "false",
        status.raw_val);
  }

  mg_printf(nc,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/json\r\n"
      "Content-Length: %d\r\n\r\n%s",
      (int)strlen(data), data);
}

static void
app_webapi_alarm_status(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  uint32_t          alarm_num;
  alarm_status_t    status;
  char              data[128];

  alarm_num = (uint32_t)app_web_get_int(subcmd);

  TRACE(APP_WEB, "alarm status request for %d\n", alarm_num);

  if(alarm_manager_get_alarm_status(alarm_num, &status) == -1)
  {
    app_webapi_not_found(nc, hm);
    return;
  }

  sprintf(data, "{ \"state\": \"%s\" }", alarm_get_string_state(status.state));

  mg_printf(nc,
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/json\r\n"
      "Content-Length: %d\r\n\r\n%s",
      (int)strlen(data), data);
}

///////////////////////////////////////////////////////////////////////////////
//
// API command declarations
//
///////////////////////////////////////////////////////////////////////////////
static struct mg_str    _api_v1_root = MG_MK_STR("/api/v1/");

static struct mg_str    _op_get   = MG_MK_STR("GET"),
                        _op_put   = MG_MK_STR("PUT"),
                        _op_post  = MG_MK_STR("POST");

static api_cmd_handler_t    _channel_cmd_handlers[] =
{
  {
    .prefix   = MG_MK_STR("status/"),
    .handler  = app_webapi_channel_status,
  },
};

static api_cmd_handler_t  _alarm_cmd_handlers[] = 
{
  {
    .prefix   = MG_MK_STR("status/"),
    .handler  = app_webapi_alarm_status,
  },
};

static api_cmd_handler_t    _top_level_get_handlers[] =
{
  {
    .prefix   = MG_MK_STR("channel/"),
    .sub      = _channel_cmd_handlers,
    .n_sub    = NARRAY(_channel_cmd_handlers),
  },
  {
    .prefix   = MG_MK_STR("alarm/"),
    .sub      = _alarm_cmd_handlers,
    .n_sub    = NARRAY(_alarm_cmd_handlers),
  },
};

///////////////////////////////////////////////////////////////////////////////
//
// API handling entry
//
///////////////////////////////////////////////////////////////////////////////
bool
app_webapi_handler(struct mg_connection* nc, struct http_message* hm)
{
  struct mg_str     cmd;

  if(app_web_has_prefix(&hm->uri, &_api_v1_root) == false)
  {
    return false;
  }

  cmd.p   = hm->uri.p     + _api_v1_root.len;
  cmd.len = hm->uri.len   - _api_v1_root.len;

  if(app_web_is_equal(&hm->method, &_op_get))
  {
    execute_api_handler(_top_level_get_handlers, NARRAY(_top_level_get_handlers), nc, hm, &cmd);
  }
  else if(app_web_is_equal(&hm->method, &_op_put))
  {
    app_webapi_not_found(nc, hm);
  }
  else if(app_web_is_equal(&hm->method, &_op_post))
  {
    app_webapi_not_found(nc, hm);
  }
  else
  {
    app_webapi_not_found(nc, hm);
  }
  return true;
}

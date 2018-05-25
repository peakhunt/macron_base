#include "common_def.h"
#include "channel_manager.h"
#include "alarm_manager.h"
#include "cfg_mgr.h"
#include "time_util.h"
#include "trace.h"
#include "webserver_api.h"
#include "json_util.h"

struct __api_cmd_handler_t;
typedef struct __api_cmd_handler_t api_cmd_handler_t;

struct __api_cmd_handler_t
{
  struct mg_str       prefix;
  bool                exact;
  api_cmd_handler_t*  sub;
  int                 n_sub;
  void (*handler)(struct mg_connection* nc, struct http_message* hm, struct mg_str* prev);
};

/*
   API List

   GET /api/v1/config/base
       base system configuration in JSON format

   GET /api/v1/channel/status/<channel_num>
       {
          "chnl_num": number,
          "eng_val": xxx or true or false,
          "raw_val": xxx
       }

   GET /api/v1/channel/status_ranged?start=xxx&end=xxx
      {
        "channels": [
           {
              "chnl_num": number,
              "eng_val": xxx or true or false,
              "raw_val": xxx
           },
           {
              "chnl_num": number,
              "eng_val": xxx or true or false,
              "raw_val": xxx
           },
           ...
        ]
      }

   GET /api/v1/alarm/status/<alarm_num>
      {
        "alarm_num": xxx,
        "state":    "inactive" or
                    "pending" or
                    "inactive_pending" or
                    "active",
        "occur_time": xxx
      }

   GET /api/v1/alarm/status_ranged?start=xxx&end=xxx
      {
        "alarms": [
           {
              "alarm_num": xxx,
              "state": "inacitve" or
                       "pending" or
                       "inactive_pending" or
                       "active",
              "occur_time": xxx
           },
           {
              "alarm_num": xxx,
              "state": "inacitve" or
                       "pending" or
                       "inactive_pending" or
                       "active",
              "occur_time": xxx
           }
           ...
        ]
      }


   POST /api/v1/channel/update/config/<chnl_num>
    request
    {
      "init_val": true/false or number,
      "failsafe_val": true/false or number,
      // only for analog channel
      "min_val": number,
      "max_val": number
    }

   POST /api/v1/alarm/update/config/<alarm_num>
    request
    {
      "set_point": true/false or number,
      "delay": number
    }

   POST /api/v1/channel/update/lookup_table/<chnl_num>
    request
    {
      "lookup_table": [
        {   "raw": number, "eng": number },
        {   "raw": number, "eng": number },
        {   "raw": number, "eng": number }
        ...
      ]
    }

   POST /api/v1/alarm/ack
    request
    {
      "alarm_num": xxx,
    }

    response
    {
      "alarm_num": xxx,
      "state":    "inactive" or
                  "pending" or
                  "inactive_pending" or
                  "active",
      "occur_time": xxx
    }
*/

///////////////////////////////////////////////////////////////////////////////
//
// common error handler
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// common utilities
//
///////////////////////////////////////////////////////////////////////////////

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
  bool                  match;

  for(int i = 0; i < num_entries; i++)
  {
    if(table[i].exact)
    {
      match = mg_util_is_equal(current, &table[i].prefix);
    }
    else
    {
      match = mg_util_has_prefix(current, &table[i].prefix);
    }

    if(match)
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

  TRACE(WEBS_DRIVER, "can't find target handler: %s\n", mg_util_to_c_str(current));
  webapi_not_found(nc, hm);
}

///////////////////////////////////////////////////////////////////////////////
//
// API command handlers
//
///////////////////////////////////////////////////////////////////////////////
static void
webapi_get_channel_status(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  uint32_t            chnl_num;
  channel_status_t    status;
  char                data[128];

  chnl_num = (uint32_t)mg_util_get_int(subcmd);

  TRACE(WEBS_REQUEST, "channel status request for %d\n", chnl_num);

  if(channel_manager_get_channel_stat(chnl_num, &status) == -1)
  {
    webapi_not_found(nc, hm);
    return;
  }

  if(status.chnl_type == channel_type_analog)
  {
    sprintf(data, "{ \"chnl_num\": %d, \"eng_val\": %.2f, \"raw_val\": %d }",
        chnl_num,
        status.eng_val.f,
        status.raw_val);
  }
  else
  {
    sprintf(data, "{ \"chnl_num\": %d, \"eng_val\": %s, \"raw_val\": %d }",
        chnl_num,
        status.eng_val.b ? "true" : "false",
        status.raw_val);
  }

  webapi_server_json_response_ok(nc, data, strlen(data));
}

static void
webapi_get_channel_status_ranged(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  char*         qstr;
  uint32_t      start,end,
                chnl_num;
  int           ndx;
  channel_status_t    status;
  bool comma_needed = false;

  qstr = mg_util_to_c_str_alloc(&hm->query_string);
  TRACE(WEBS_REQUEST, "channel status ranged request %s\n", qstr);

  if(sscanf(qstr, "start=%d&end=%d", &start, &end) != 2 || end < start)
  {
    webapi_bad_request(nc,hm);
    goto out;
  }

  TRACE(WEBS_REQUEST, "channel status ranged request %d, %d\n", start, end);
  mg_printf(nc, "%s",
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/json\r\n"
      "Transfer-Encoding: chunked\r\n\r\n");
  mg_printf_http_chunk(nc, "{\"channels\": [");

  ndx = cfg_mgr_get_channel_index_equal_or_bigger(start);
  while(ndx != -1)
  {
    chnl_num = cfg_mgr_get_channel_from_index(ndx);
    if(chnl_num > end)
    {
      break;
    }

    if(comma_needed)
    {
      mg_printf_http_chunk(nc, ",");
    }

    channel_manager_get_channel_stat(chnl_num, &status);

    if(status.chnl_type == channel_type_analog)
    {
      mg_printf_http_chunk(nc, "{\"chnl_num\": %d, \"eng_val\": %.2f, \"raw_val\": %d }",
          chnl_num,
          status.eng_val.f,
          status.raw_val);
    }
    else
    {
      mg_printf_http_chunk(nc, "{ \"chnl_num\": %d, \"eng_val\": %s, \"raw_val\": %d }",
          chnl_num,
          status.eng_val.b ? "true" : "false",
          status.raw_val);
    }
    comma_needed = true;
    ndx = cfg_mgr_get_channel_next(ndx);
  }

  mg_printf_http_chunk(nc, "]}");
  mg_send_http_chunk(nc, "", 0);

out:
  free(qstr);
}

static void
webapi_update_channel_config(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  uint32_t                    chnl_num;
  cJSON*                      req;
  channel_runtime_config_t    cfg;

  const static json_util_field_t           _param_type1[] = 
  {
    { "init_val",     cJSON_False | cJSON_True      },
    { "failsafe_val", cJSON_False | cJSON_True      },
  };
  const static json_util_field_t           _param_type2[] = 
  {
    { "init_val",     cJSON_Number                  },
    { "failsafe_val", cJSON_Number                  },
    { "min_val",      cJSON_Number                  },
    { "max_val",      cJSON_Number                  },
  };

  chnl_num = (uint32_t)mg_util_get_int(subcmd);

  TRACE(WEBS_REQUEST, "channel config update request for %d\n", chnl_num);

  req = webapi_parse_json_body(&hm->body, nc, hm);
  if(req == NULL)
  {
    return;
  }

  if(json_util_simple_validate_message(req, _param_type1, NARRAY(_param_type1)))
  {   
    cfg.init_value.b      = cJSON_GetObjectItem(req, "init_val")->valueint;
    cfg.failsafe_value.b  = cJSON_GetObjectItem(req, "failsafe_val")->valueint;
  }
  else if(json_util_simple_validate_message(req, _param_type2, NARRAY(_param_type2)))
  {   
    cfg.init_value.f      = cJSON_GetObjectItem(req, "init_val")->valuedouble;
    cfg.failsafe_value.f  = cJSON_GetObjectItem(req, "failsafe_val")->valuedouble;
    cfg.min_val           = cJSON_GetObjectItem(req, "min_val")->valuedouble;
    cfg.max_val           = cJSON_GetObjectItem(req, "max_val")->valuedouble;
  }
  else
  {
    webapi_bad_request(nc, hm);
    goto out;
  }

  if(cfg_mgr_update_channel_cfg(chnl_num, &cfg) == TRUE)
  {
    webapi_server_response_ok(nc);
  }
  else
  {
    webapi_server_error_other(nc, hm);
  }

out:
  cJSON_Delete(req);
}

static void
webapi_update_alarm_config(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  uint32_t                    alarm_num;
  cJSON*                      req;
  alarm_runtime_config_t      cfg;

  const static json_util_field_t           _param_type1[] = 
  {
    { "set_point",    cJSON_False | cJSON_True      },
    { "delay",        cJSON_Number                  },
  };
  const static json_util_field_t           _param_type2[] = 
  {
    { "set_point",    cJSON_Number                  },
    { "delay",        cJSON_Number                  },
  };

  alarm_num = (uint32_t)mg_util_get_int(subcmd);

  TRACE(WEBS_REQUEST, "alarm config update request for %d\n", alarm_num);

  req = webapi_parse_json_body(&hm->body, nc, hm);
  if(req == NULL)
  {
    TRACE(ERROR, "alarm config bad request. json parse failed %d\n", alarm_num);
    webapi_bad_request(nc, hm);
    return;
  }

  if(json_util_simple_validate_message(req, _param_type1, NARRAY(_param_type1)))
  {   
    cfg.set_point.b       = cJSON_GetObjectItem(req, "set_point")->valueint;
    cfg.delay             = cJSON_GetObjectItem(req, "delay")->valuedouble;
  }
  else if(json_util_simple_validate_message(req, _param_type2, NARRAY(_param_type2)))
  {   
    cfg.set_point.f       = cJSON_GetObjectItem(req, "set_point")->valuedouble;
    cfg.delay             = cJSON_GetObjectItem(req, "delay")->valuedouble;
  }
  else
  {
    TRACE(ERROR, "alarm config bad request. parameter missing %d\n", alarm_num);
    webapi_bad_request(nc, hm);
    goto out;
  }

  if(cfg_mgr_update_alarm_cfg(alarm_num, &cfg) == TRUE)
  {
    webapi_server_response_ok(nc);
  }
  else
  {
    webapi_server_error_other(nc, hm);
  }

out:
  cJSON_Delete(req);
}

static void
webapi_ack_alarm(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  cJSON*            req;
  uint32_t          alarm_num;
  const static json_util_field_t           _param_type1[] = 
  {
    { "alarm_num",    cJSON_Number                  },
  };

  req = webapi_parse_json_body(&hm->body, nc, hm);
  if(req == NULL)
  {
    return;
  }


  if(json_util_simple_validate_message(req, _param_type1, NARRAY(_param_type1)))
  {
    alarm_num = (uint32_t)cJSON_GetObjectItem(req, "alarm_num")->valueint;

    TRACE(WEBS_REQUEST, "alarm ack request for %d\n", alarm_num);

    if(alarm_manager_ack_alarm(alarm_num) == TRUE)
    {
      alarm_status_t    status;
      char              data[128];

      alarm_manager_get_alarm_status(alarm_num, &status);
      sprintf(data, "{ \"alarm_num\": %d, \"state\": \"%s\", \"occur_time\": %ld }",
          alarm_num,
          alarm_get_string_state(status.state),
          status.occur_time);
      webapi_server_json_response_ok(nc, data, strlen(data));
    }
    else
    {
      webapi_bad_request(nc, hm);
      goto out;
    }
  }
  else
  {
    webapi_bad_request(nc, hm);
    goto out;
  }

out:
  cJSON_Delete(req);
}

static void
webapi_update_channel_lookup_table(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  uint32_t                    chnl_num;
  cJSON                       *req,
                              *ltable,
                              *entry;
  int                         num_entries;
  lookup_table_item_t*        items;
  lookup_table_t*             lookup_table;

  const static json_util_field_t      _param_req1[] = 
  {
    { "lookup_table",     cJSON_Array                  },
  };

  const static json_util_field_t    _param_req2[] =
  {
    { "raw",     cJSON_Number                  },
    { "eng",     cJSON_Number                  },
  };

  chnl_num = (uint32_t)mg_util_get_int(subcmd);

  TRACE(WEBS_REQUEST, "lookup table update request for %d\n", chnl_num);

  req = webapi_parse_json_body(&hm->body, nc, hm);
  if(req == NULL)
  {
    webapi_bad_request(nc, hm);
    return;
  }

  if(json_util_simple_validate_message(req, _param_req1, NARRAY(_param_req1)) == FALSE)
  {
    webapi_bad_request(nc, hm);
    goto out;
  }

  ltable = cJSON_GetObjectItem(req, "lookup_table");
  num_entries = cJSON_GetArraySize(ltable);

  if(num_entries != 0)
  {
    for(int i = 0; i < num_entries; i++)
    {
      entry = cJSON_GetArrayItem(ltable, i);
      if(json_util_simple_validate_message(entry, _param_req2, NARRAY(_param_req2)) == FALSE)
      {
        webapi_bad_request(nc, hm);
        goto out;
      }
    }

    items = malloc(sizeof(lookup_table_item_t) * num_entries);

    for(int i = 0; i < num_entries; i++)
    {
      entry = cJSON_GetArrayItem(ltable, i);
      items[i].v1 = cJSON_GetObjectItem(entry, "raw")->valuedouble;
      items[i].v2 = cJSON_GetObjectItem(entry, "eng")->valuedouble;
    }

    lookup_table = malloc(sizeof(lookup_table_t));
    lookup_table_init(lookup_table, items, num_entries);
    free(items);
  }
  else
  {
    lookup_table = NULL;
  }

  if(cfg_mgr_update_lookup_table(chnl_num, lookup_table) == TRUE)
  {
    webapi_server_response_ok(nc);
  }
  else
  {
    webapi_server_error_other(nc, hm);
  }

out:
  cJSON_Delete(req);
}

static void
webapi_get_alarm_status(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  uint32_t          alarm_num;
  alarm_status_t    status;
  char              data[128];

  alarm_num = (uint32_t)mg_util_get_int(subcmd);

  TRACE(WEBS_REQUEST, "alarm status request for %d\n", alarm_num);

  if(alarm_manager_get_alarm_status(alarm_num, &status) == -1)
  {
    webapi_not_found(nc, hm);
    return;
  }

  sprintf(data, "{ \"alarm_num\": %d, \"state\": \"%s\", \"occur_time\": %ld }",
      alarm_num,
      alarm_get_string_state(status.state),
      status.occur_time);
  webapi_server_json_response_ok(nc, data, strlen(data));
}

static void
webapi_get_alarm_status_ranged(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  char*         qstr;
  uint32_t      start, end,
                alarm_num;
  int           ndx;
  alarm_status_t    status;
  bool comma_needed = false;

  qstr = mg_util_to_c_str_alloc(&hm->query_string);
  TRACE(WEBS_REQUEST, "alarm status ranged request %s\n", qstr);

  if(sscanf(qstr, "start=%d&end=%d", &start, &end) != 2 || end < start)
  {
    webapi_bad_request(nc,hm);
    goto out;
  }

  TRACE(WEBS_REQUEST, "alarm status ranged request %d, %d\n", start, end);
  mg_printf(nc, "%s",
      "HTTP/1.1 200 OK\r\n"
      "Content-Type: text/json\r\n"
      "Transfer-Encoding: chunked\r\n\r\n");
  mg_printf_http_chunk(nc, "{\"alarms\": [");

  ndx = cfg_mgr_get_alarm_index_equal_or_bigger(start);
  while(ndx != -1)
  {
    alarm_num = cfg_mgr_get_alarm_from_index(ndx);
    if(alarm_num > end)
    {
      break;
    }

    if(comma_needed)
    {
      mg_printf_http_chunk(nc, ",");
    }
    
    alarm_manager_get_alarm_status(alarm_num, &status);

    mg_printf_http_chunk(nc, "{ \"alarm_num\": %d, \"state\": \"%s\", \"occur_time\": %ld }",
        alarm_num,
        alarm_get_string_state(status.state),
        status.occur_time);

    comma_needed = true;
    ndx = cfg_mgr_get_alarm_next(ndx);
  }

  mg_printf_http_chunk(nc, "]}");
  mg_send_http_chunk(nc, "", 0);

out:
  free(qstr);
}

static void
webapi_get_config_base(struct mg_connection* nc, struct http_message* hm, struct mg_str* subcmd)
{
  const char*   json;
  int     len;

  cfg_mgr_read_lock();

  json = cfg_mgr_get_json_string_use_lock_before_call(&len);

  if(json == NULL)
  {
    webapi_server_error(nc, hm);
  }
  else
  {
    webapi_server_json_response_ok(nc, json, len);
  }
  cfg_mgr_unlock();
}


///////////////////////////////////////////////////////////////////////////////
//
// API command declarations
//
///////////////////////////////////////////////////////////////////////////////
static struct mg_str    _api_v1_root = MG_MK_STR("/api/v1/");

static api_cmd_handler_t    _channel_cmd_handlers[] =
{
  {
    .prefix   = MG_MK_STR("status/"),
    .handler  = webapi_get_channel_status,
  },
  {
    .prefix   = MG_MK_STR("status_ranged"),
    .exact    = true,
    .handler  = webapi_get_channel_status_ranged,
  },
};

static api_cmd_handler_t  _alarm_cmd_handlers[] = 
{
  {
    .prefix   = MG_MK_STR("status/"),
    .handler  = webapi_get_alarm_status,
  },
  {
    .prefix   = MG_MK_STR("status_ranged"),
    .exact    = true,
    .handler  = webapi_get_alarm_status_ranged,
  },
};

static api_cmd_handler_t  _config_handlers[] =
{
  {
    .prefix   = MG_MK_STR("base"),
    .exact    = true,
    .handler  = webapi_get_config_base,
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
  {
    .prefix   = MG_MK_STR("config/"),
    .sub      = _config_handlers,
    .n_sub    = NARRAY(_config_handlers),
  },
};

static api_cmd_handler_t  _top_level_post_handlers[] =
{
  {
    .prefix   = MG_MK_STR("channel/update/config/"),
    .handler  = webapi_update_channel_config,
  },
  {
    .prefix   = MG_MK_STR("channel/update/lookup_table/"),
    .handler  = webapi_update_channel_lookup_table,
  },
  {
    .prefix   = MG_MK_STR("alarm/update/config/"),
    .handler  = webapi_update_alarm_config,
  },
  {
    .prefix   = MG_MK_STR("alarm/ack"),
    .exact    = true,
    .handler  = webapi_ack_alarm,
  }
};

///////////////////////////////////////////////////////////////////////////////
//
// API handling entry
//
///////////////////////////////////////////////////////////////////////////////
bool
webserver_api_handler(struct mg_connection* nc, struct http_message* hm)
{
  struct mg_str     cmd;
  unsigned long     start;

  start = time_util_get_sys_clock_in_ms();

  if(mg_util_has_prefix(&hm->uri, &_api_v1_root) == false)
  {
    return false;
  }

  cmd.p   = hm->uri.p     + _api_v1_root.len;
  cmd.len = hm->uri.len   - _api_v1_root.len;

  if(mg_util_is_equal(&hm->method, &_op_get))
  {
    execute_api_handler(_top_level_get_handlers, NARRAY(_top_level_get_handlers), nc, hm, &cmd);
  }
  else if(mg_util_is_equal(&hm->method, &_op_put))
  {
    webapi_not_found(nc, hm);
  }
  else if(mg_util_is_equal(&hm->method, &_op_post))
  {
    execute_api_handler(_top_level_post_handlers, NARRAY(_top_level_post_handlers), nc, hm, &cmd);
  }
  else
  {
    webapi_not_found(nc, hm);
  }

  TRACE(WEBS_REQUEST, "done handling api request. took %ld ms\n",
      time_util_get_sys_clock_elapsed_in_ms(start));

  return true;
}

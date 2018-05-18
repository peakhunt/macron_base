#include "common_def.h"
#include "channel_manager.h"
#include "alarm_manager.h"
#include "cfg_mgr.h"
#include "time_util.h"
#include "trace.h"
#include "webserver_api_loggerif.h"
#include "json_util.h"

#include "logger.h"
#include "logger_sql3_common.h"

static struct mg_str    _api_v1_loggerif_root = MG_MK_STR("/api/v1/loggerif/");
static struct mg_str    _channel_prefix   = MG_MK_STR("channel");
static struct mg_str    _alarm_prefix     = MG_MK_STR("alarm");
static struct mg_str    _chn_trace_prefix = MG_MK_STR("channel/trace");

static sqlite3* _logger_db  = NULL;

typedef struct
{
  struct mg_connection*     nc;
  struct http_message*      hm;
  bool                      first;
  uint32_t                  count;
} get_ctx_t;


/*
   API List

   GET /api/vi/loggerif/channel?start_time=xxx&end_time=xxx&ch=xx,xx,xx,xx
   => 
   select * from channel_log
   where ((ch_num = 1 or ch_num = 2) and (time_stamp >= 1822310417 and time_stamp <= 1822312615))
   order by ch_num asc, time_stamp asc;

   response
   {
     "data": [
        { "ch": XXX, "data": XXX, "timestamp": XXX, },
        { "ch": XXX, "data": XXX, "timestamp": XXX, },
        { "ch": XXX, "data": XXX, "timestamp": XXX, },
        { "ch": XXX, "data": XXX, "timestamp": XXX, },
        { "ch": XXX, "data": XXX, "timestamp": XXX, },
        ...
      ]
   }

   GET /api/vi/loggerif/alarm?start_time=xxx&end_time=xxx
   =>
   select * from alarm_log
   where time_stamp >= start_time and time_stamp < end_time
   order by time_stamp asc
   {
     "alarms": [
      { "alarm": XXX, "timestamp": XXX, "event": XXX },
      { "alarm": XXX, "timestamp": XXX, "event": XXX },
      { "alarm": XXX, "timestamp": XXX, "event": XXX },
      { "alarm": XXX, "timestamp": XXX, "event": XXX },
      ...
     ]
   }

   GET  /api/v1/loggerif/channel/trace
   response
   {
     "channels" = [
      XXX,
      XXX,
      XXX
     ]
   }

   POST /api/v1/loggerif/channel/trace
   request
   {
     "channels" = [
      XXX,
      XXX,
      XXX
     ]
   }
*/

static int
get_channel_log_callback(void* data, int argc, char** argv, char** azColName)
{
  get_ctx_t* ctx = (get_ctx_t*)data;
  struct mg_connection* nc = ctx->nc;
  bool first = ctx->first;

  if(ctx->first)
  {
    mg_printf(nc, "%s",
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/json\r\n"
        "Transfer-Encoding: chunked\r\n\r\n");

    mg_printf_http_chunk(nc, "{\"data\": [");
    ctx->first = false;
  }

  if(!first)
  {
    mg_printf_http_chunk(nc, ",");
  }

  mg_printf_http_chunk(nc, "{\"ch\": %s, \"data\": %s, \"timestamp\": %s }",
      argv[0], argv[2], argv[1]);
  ctx->count++;

  return 0;
}

static void
webapi_get_channel_log(struct mg_connection* nc, struct http_message* hm)
{
  char*         qstr;
  char          chnl_list_str[512];
  unsigned long start_time,
                end_time;
  char          *s, *t;
  uint32_t      chnls[16];
  uint32_t      num_chnls = 0;

  get_ctx_t ctx = 
  {
    .nc = nc,
    .hm = hm,
    .first = true,
    .count = 0,
  };

  qstr = mg_util_to_c_str_alloc(&hm->query_string);

  TRACE(WEBL_REQUEST, "webapi_get_channel_log: %s\n", qstr);

  // FIXME
  // this code is dangerous. maybe next time when have some free time.
  // -hkim-
  //
  if(sscanf(qstr, "start_time=%lu&end_time=%lu&ch=%s", &start_time, &end_time, chnl_list_str) != 3)
  {
    TRACE(WEBL_REQUEST, "sscanf error\n");
    webapi_bad_request(nc,hm);
    goto out;
  }

  TRACE(WEBL_REQUEST, "%lu, %lu, %s\n", start_time, end_time, chnl_list_str);

  while((s = strtok_r(num_chnls == 0 ? chnl_list_str : NULL, ",", &t)) != NULL)
  {
    if(num_chnls >= 16)
    {
      webapi_bad_request(nc, hm);
      goto out;
    }

    chnls[num_chnls++] = atoi(s);
  }

  (void)chnls;
  for(uint32_t i = 0; i < num_chnls; i++)
  {
    TRACE(WEBL_REQUEST, "CH - %u\n", chnls[i]);
  }

  if(logger_db_get_channel_log(_logger_db, start_time, end_time, chnls, num_chnls, get_channel_log_callback, &ctx))
  {
    if(ctx.first)
    {
      // now rows at all
      mg_printf(nc, "%s",
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/json\r\n"
          "Transfer-Encoding: chunked\r\n\r\n");
      mg_printf_http_chunk(nc, "{\"data\": [");
    }

    mg_printf_http_chunk(nc, "]}");
    mg_send_http_chunk(nc, "", 0);

    TRACE(WEBL_REQUEST, "channe log returned %u data\n", ctx.count);
  }
  else
  {
    webapi_server_error_other(nc, hm);
  }

out:
  free(qstr);
}

static int
get_alarm_log_callback(void* data, int argc, char** argv, char** azColName)
{
  get_ctx_t* ctx = (get_ctx_t*)data;
  struct mg_connection* nc = ctx->nc;
  bool first = ctx->first;

  if(ctx->first)
  {
    mg_printf(nc, "%s",
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/json\r\n"
        "Transfer-Encoding: chunked\r\n\r\n");

    mg_printf_http_chunk(nc, "{\"alarms\": [");
    ctx->first = false;
  }

  if(!first)
  {
    mg_printf_http_chunk(nc, ",");
  }

  mg_printf_http_chunk(nc, "{\"alarm\": %s, \"timestamp\": %s, \"event\": %s }",
      argv[0], argv[1], argv[2]);
  ctx->count++;

  return 0;
}

static void
webapi_get_alarm_log(struct mg_connection* nc, struct http_message* hm)
{
  char*         qstr;
  unsigned long start_time,
                end_time;

  get_ctx_t ctx = 
  {
    .nc = nc,
    .hm = hm,
    .first = true,
    .count = 0,
  };

  qstr = mg_util_to_c_str_alloc(&hm->query_string);

  // FIXME
  // this code is dangerous. maybe next time when have some free time.
  // -hkim-
  //
  if(sscanf(qstr, "start_time=%lu&end_time=%lu", &start_time, &end_time) != 2)
  {
    TRACE(WEBL_REQUEST, "sscanf error\n");
    webapi_bad_request(nc,hm);
    goto out;
  }

  TRACE(WEBL_REQUEST, "%lu, %lu\n", start_time, end_time);

  if(logger_db_get_alarm_log(_logger_db, start_time, end_time, get_alarm_log_callback, &ctx))
  {
    if(ctx.first)
    {
      // no rows at all
      mg_printf(nc, "%s",
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/json\r\n"
          "Transfer-Encoding: chunked\r\n\r\n");

      mg_printf_http_chunk(nc, "{\"alarms\": [");
    }
    mg_printf_http_chunk(nc, "]}");
    mg_send_http_chunk(nc, "", 0);

    TRACE(WEBL_REQUEST, "alarm log returned %u data\n", ctx.count);
  }
  else
  {
    TRACE(WEBL_REQUEST, "fucking error!!!\n");
    webapi_server_error_other(nc, hm);
  }

out:
  free(qstr);
}

static int
get_trace_channel_callback(void* data, int argc, char** argv, char** azColName)
{
  get_ctx_t* ctx = (get_ctx_t*)data;
  struct mg_connection* nc = ctx->nc;
  bool first = ctx->first;

  if(ctx->first)
  {
    mg_printf(nc, "%s",
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/json\r\n"
        "Transfer-Encoding: chunked\r\n\r\n");

    mg_printf_http_chunk(nc, "{\"channels\": [");
    ctx->first = false;
  }

  if(!first)
  {
    mg_printf_http_chunk(nc, ",");
  }

  mg_printf_http_chunk(nc, "%s", argv[0]);
  ctx->count++;

  return 0;
}

static void
webapi_get_channel_trace(struct mg_connection* nc, struct http_message* hm)
{
  get_ctx_t ctx = 
  {
    .nc = nc,
    .hm = hm,
    .first = true,
    .count = 0,
  };

  if(logger_db_get_all_trace_channels(_logger_db,  get_trace_channel_callback, &ctx))
  {
    if(ctx.first)
    {
      // now row
      mg_printf(nc, "%s",
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/json\r\n"
          "Transfer-Encoding: chunked\r\n\r\n");

      mg_printf_http_chunk(nc, "{\"channels\": [");
    }
    mg_printf_http_chunk(nc, "]}");
    mg_send_http_chunk(nc, "", 0);

    TRACE(WEBL_REQUEST, "trace channel returned %u data\n", ctx.count);
  }
  else
  {
    webapi_server_error_other(nc, hm);
  }
}

static void
webapi_set_channel_trace(struct mg_connection* nc, struct http_message* hm)
{
  cJSON     *jreq,
            *jchnls;
  uint32_t  *chnls = NULL,
            num_chnls;

  jreq = webapi_parse_json_body(&hm->body, nc, hm);
  if(jreq == NULL)
  {
    webapi_bad_request(nc, hm);
    return;
  }

  // validate
  // channels must be present and it should be an array of int
  //
  jchnls = cJSON_GetObjectItem(jreq, "channels");
  if(jchnls == NULL || !(jchnls->type & cJSON_Array))
  {
    webapi_bad_request(nc, hm);
    goto out;
  }

  num_chnls = cJSON_GetArraySize(jchnls);
  if(num_chnls > 0)
  {
    chnls = malloc(sizeof(uint32_t)*num_chnls);
    if(chnls == NULL)
    {
      webapi_server_error_other(nc, hm);
      goto out;
    }
    for(uint32_t ndx = 0; ndx < num_chnls; ndx++)
    {
      chnls[ndx] = (uint32_t)cJSON_GetArrayItem(jchnls, ndx)->valueint;
    }
  }
  else
  {
    chnls = NULL;
  }

  logger_signal_trace_set_chnls(chnls, num_chnls);
  webapi_server_response_ok(nc);

out:
  if(chnls != NULL)
  {
    free(chnls);
  }
  cJSON_Delete(jreq);
}

bool
webserver_api_loggerif_handler(struct mg_connection* nc, struct http_message* hm)
{
  struct mg_str     cmd;
  unsigned long     start;

  start = time_util_get_sys_clock_in_ms();

  if(mg_util_has_prefix(&hm->uri, &_api_v1_loggerif_root) == false)
  {
    return false;
  }

  cmd.p   = hm->uri.p     + _api_v1_loggerif_root.len;
  cmd.len = hm->uri.len   - _api_v1_loggerif_root.len;

  if(mg_util_is_equal(&hm->method, &_op_get))
  {
    if(mg_util_is_equal(&cmd, &_channel_prefix))
    {
      webapi_get_channel_log(nc, hm);
    }
    else if(mg_util_is_equal(&cmd, &_alarm_prefix))
    {
      webapi_get_alarm_log(nc, hm);
    }
    else if(mg_util_is_equal(&cmd, &_chn_trace_prefix))
    {
      webapi_get_channel_trace(nc, hm);
    }
    else
    {
      webapi_not_found(nc, hm);
    }
  }
  else if(mg_util_is_equal(&hm->method, &_op_post))
  {
    if(mg_util_is_equal(&cmd, &_chn_trace_prefix))
    {
      webapi_set_channel_trace(nc, hm);
    }
    else
    {
      webapi_not_found(nc, hm);
    }
  }
  else
  {
    webapi_not_found(nc, hm);
  }

  (void)cmd;

  TRACE(WEBL_REQUEST, "done handling loggerif api request. took %ld ms\n",
      time_util_get_sys_clock_elapsed_in_ms(start));

  return FALSE;
}

void
webserver_api_loggerif_init(void)
{
  _logger_db = logger_db_open(LOGGER_DB);
  if(_logger_db == NULL)
  {
    TRACE(WEBL_REQUEST, "failed to open logger database\n");
    exit(-1);
  }
}

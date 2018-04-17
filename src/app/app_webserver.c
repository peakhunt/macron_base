#include <pthread.h>
#include "app_webserver.h"
#include "trace.h"
#include "mongoose.h"

#define WEB_SERVER_PORT         "8000"

///////////////////////////////////////////////////////////////////////////////
//
// module privates
//
///////////////////////////////////////////////////////////////////////////////
static pthread_t    _mongoose_thread;
static struct mg_serve_http_opts s_http_server_opts;

static struct mg_str    _api_v1_root = MG_MK_STR("/api/v1/");

///////////////////////////////////////////////////////////////////////////////
//
// utilities
//
///////////////////////////////////////////////////////////////////////////////
static inline const char*
c_str_from_mg_str(struct mg_str ms)
{
  static char str[1024 + 1];
  int   len;

  len = ms.len <= 1024 ? ms.len : 1024;
  strncpy(str, ms.p, len);
  str[len] = '\0';

  return str;
}

///////////////////////////////////////////////////////////////////////////////
//
// JSON REST API Handler
//
///////////////////////////////////////////////////////////////////////////////
static void
__handle_api_v1_request(struct mg_connection* nc, struct http_message* hm)
{
  // FIXME
}

///////////////////////////////////////////////////////////////////////////////
//
// mongoose event handler
//
///////////////////////////////////////////////////////////////////////////////
static void
ev_handler(struct mg_connection* nc, int ev, void* ev_data)
{
  struct http_message* hm = (struct http_message*)ev_data;

  switch(ev)
  {
  case MG_EV_HTTP_REQUEST:
    if(mg_strncmp(hm->uri, _api_v1_root, _api_v1_root.len) == 0)
    {
      TRACE(APP_WEB, "api request: %s\n", c_str_from_mg_str(hm->uri));
      __handle_api_v1_request(nc, hm);
    }
    else if(mg_vcmp(&hm->uri, "/printcontent") == 0) 
    { // just for a test
      char buf[100] = {0};

      memcpy(buf, hm->body.p, sizeof(buf) - 1 < hm->body.len ? sizeof(buf) - 1 : hm->body.len);
      TRACE(APP_WEB, "printing contents\n");
      TRACE(APP_WEB, "%s\n", buf);
    }
    else
    {
      mg_serve_http(nc, hm, s_http_server_opts);
    }
    break;

  default:
    break;
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// mongoose thread
//
///////////////////////////////////////////////////////////////////////////////
static void*
__mongoose_thread(void* arg)
{
  struct mg_mgr           mgr;
  struct mg_connection*   nc;

  TRACE(APP_WEB, "starting web server for port %s\n", WEB_SERVER_PORT);
  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, WEB_SERVER_PORT, ev_handler);
  if(nc == NULL)
  {
    TRACE(APP_WEB, "failed to create listener\n");
    return NULL;
  }

  mg_set_protocol_http_websocket(nc);

  s_http_server_opts.document_root = ".";
  s_http_server_opts.enable_directory_listing = "yes";

  for(;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// publics
//
///////////////////////////////////////////////////////////////////////////////
void
app_webserver_init(void)
{
  pthread_create(&_mongoose_thread, NULL, __mongoose_thread, NULL);
}

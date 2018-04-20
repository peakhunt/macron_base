#include <pthread.h>
#include "webserver_driver.h"
#include "trace.h"
#include "mongoose.h"
#include "app_init_completion.h"
#include "completion.h"
#include "mongoose_util.h"
#include "webserver_api.h"

#define WEB_SERVER_PORT         "8000"

///////////////////////////////////////////////////////////////////////////////
//
// module privates
//
///////////////////////////////////////////////////////////////////////////////
static pthread_t    _mongoose_thread;
static struct mg_serve_http_opts s_http_server_opts;

static completion_t     _go_signal;

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
    if(webserver_api_handler(nc, hm))
    {
      return;
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

  TRACE(APP_WEB, "done starting web server\n");
  app_init_complete_signal();

  TRACE(APP_WEB, "waiting for go signal from main\n");
  completion_wait(&_go_signal);
  TRACE(APP_WEB, "got go signal from main. entering main loop\n");

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
webserver_driver_init(void)
{
  TRACE(APP_WEB, "starting web server interface\n");

  completion_init(&_go_signal);

  pthread_create(&_mongoose_thread, NULL, __mongoose_thread, NULL);
  app_init_complete_wait();
}

void
webserver_driver_go(void)
{
  completion_signal(&_go_signal);
}

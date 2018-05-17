#include <pthread.h>
#include "webserver_driver.h"
#include "cfg_mgr.h"
#include "trace.h"
#include "mongoose.h"
#include "app_init_completion.h"
#include "completion.h"
#include "mongoose_util.h"
#include "webserver_api.h"
#include "webserver_api_loggerif.h"

// XXX
// two threads.
// thread 1 (aka mongoose thread) is for high speed realtime transaction
// thread 2 (aka logger if thread) is for high volume log data query
//
// -hkim-

///////////////////////////////////////////////////////////////////////////////
//
// module privates
//
///////////////////////////////////////////////////////////////////////////////
static pthread_t    _mongoose_thread;
static pthread_t    _mongoose_loggerif;
static struct mg_serve_http_opts s_http_server_opts;

static completion_t     _go_signal;
static completion_t     _go_signal_loggerif;

static char*            _http_address;
static char*            _doc_root;

static char*            _http_loggerif_address = "8001";
static char*            _doc_loggerif_root = "./";

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
      TRACE(WEBS_DRIVER, "printing contents\n");
      TRACE(WEBS_DRIVER, "%s\n", buf);
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

  TRACE(WEBS_DRIVER, "starting web server bound to %s, doc root %s\n", _http_address, _doc_root);
  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, _http_address, ev_handler);
  if(nc == NULL)
  {
    TRACE(WEBS_DRIVER, "failed to create listener\n");
    return NULL;
  }

  mg_set_protocol_http_websocket(nc);

  s_http_server_opts.document_root = _doc_root;
  s_http_server_opts.enable_directory_listing = "yes";

  TRACE(WEBS_DRIVER, "done starting web server\n");
  app_init_complete_signal();

  TRACE(WEBS_DRIVER, "waiting for go signal from main\n");
  completion_wait(&_go_signal);
  TRACE(WEBS_DRIVER, "got go signal from main. entering main loop\n");

  for(;;) {
    mg_mgr_poll(&mgr, 1000);
  }
  mg_mgr_free(&mgr);

  return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// mongoose event handler for logger if
//
///////////////////////////////////////////////////////////////////////////////
static void
ev_loggerif_handler(struct mg_connection* nc, int ev, void* ev_data)
{
  struct http_message* hm = (struct http_message*)ev_data;

  switch(ev)
  {
  case MG_EV_HTTP_REQUEST:
    if(webserver_api_loggerif_handler(nc, hm))
    {
      return;
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
// mongoose thread for logger if
//
///////////////////////////////////////////////////////////////////////////////
static void*
__mongoose_loggerif_thread(void* arg)
{
  struct mg_mgr           mgr;
  struct mg_connection*   nc;

  TRACE(WEBS_DRIVER, "loggerif. starting web server bound to %s, doc root %s\n", _http_loggerif_address, _doc_loggerif_root);

  webserver_api_loggerif_init();

  mg_mgr_init(&mgr, NULL);

  nc = mg_bind(&mgr, _http_loggerif_address, ev_loggerif_handler);
  if(nc == NULL)
  {
    TRACE(WEBS_DRIVER, "loggerif failed to create listener\n");
    return NULL;
  }

  mg_set_protocol_http_websocket(nc);

  s_http_server_opts.document_root = _doc_loggerif_root;
  s_http_server_opts.enable_directory_listing = "yes";

  TRACE(WEBS_DRIVER, "loggerif. done starting web server\n");
  app_init_complete_signal();

  TRACE(WEBS_DRIVER, "loggerif. waiting for go signal from main\n");
  completion_wait(&_go_signal_loggerif);
  TRACE(WEBS_DRIVER, "loggerif. got go signal from main. entering main loop\n");

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
  webserver_config_t    cfg;

  TRACE(WEBS_DRIVER, "starting web server interface\n");

  cfg_mgr_get_webserver_config(&cfg);

  _http_address = strdup(cfg.http_address);
  _doc_root     = strdup(cfg.doc_root);

  completion_init(&_go_signal);
  completion_init(&_go_signal_loggerif);

  pthread_create(&_mongoose_thread, NULL, __mongoose_thread, NULL);
  app_init_complete_wait();

  pthread_create(&_mongoose_loggerif, NULL, __mongoose_loggerif_thread, NULL);
  app_init_complete_wait();
}

void
webserver_driver_go(void)
{
  completion_signal(&_go_signal);
  completion_signal(&_go_signal_loggerif);
}

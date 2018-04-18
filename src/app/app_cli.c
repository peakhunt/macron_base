#include <stdio.h>
#include "app_cli.h"
#include "app_config.h"
#include "cli.h"
#include "trace.h"
#include "app_init_completion.h"
#include "evloop_thread.h"
#include "completion.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
static void app_cli_thread_init(evloop_thread_t* thrd);
static void app_cli_thread_fini(evloop_thread_t* thrd);

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
static evloop_thread_t    _app_cli_thread =
{
  .init = app_cli_thread_init,
  .fini = app_cli_thread_fini,
};

static completion_t     _go_signal;

////////////////////////////////////////////////////////////////////////////////
//
// thread initializer
//
////////////////////////////////////////////////////////////////////////////////
static void
app_cli_thread_init(evloop_thread_t* thrd)
{
  cli_config_t    cfg;

  TRACE(APP_CLI, "initializing CLI\n");

  app_config_get_cli_config(&cfg);
  cli_init(&cfg, NULL, 0);

  TRACE(APP_CLI, "done initializing CLI\n");

  app_init_complete_signal();

  TRACE(APP_CLI, "waiting for go signal from main\n");
  completion_wait(&_go_signal);
  TRACE(APP_CLI, "got go signal from main. entering main loop\n");
}

static void
app_cli_thread_fini(evloop_thread_t* thrd)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////

void
app_cli_init(void)
{
  TRACE(APP_CLI, "starting CLI\n");

  completion_init(&_go_signal);

  evloop_thread_init(&_app_cli_thread);
  evloop_thread_run(&_app_cli_thread);

  app_init_complete_wait();
}

void
app_cli_go(void)
{
  completion_signal(&_go_signal);
}

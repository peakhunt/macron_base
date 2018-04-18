#include <unistd.h>
#include "common_def.h"
#include "cmd_option.h"
#include "debug_log.h"
#include "utils.h"
#include "evloop_thread.h"
#include "trace.h"
#include "app_config.h"
#include "app_cli.h"
#include "app_modbus_slave.h"
#include "app_modbus_master.h"
#include "app_core.h"
#include "app_webserver.h"
#include "completion.h"

static void main_thread_init(evloop_thread_t* thrd);
static completion_t     _comp;

static uint32_t   _initial_trace_setup[] =
{
  TRACE_COMP(MAIN),
  TRACE_COMP(APP_START),
  TRACE_COMP(APP_CORE),
  //TRACE_COMP(CHANNEL),
  //TRACE_COMP(CHANNELM),
  TRACE_COMP(TEST),
  TRACE_COMP(TASK),
  TRACE_COMP(SOCK_ERR),
  TRACE_COMP(SERIAL),
  TRACE_COMP(CLI_TELNET),
  TRACE_COMP(CLI_SERIAL),
  TRACE_COMP(CLI),
  //TRACE_COMP(MB_TCP_SLAVE),
  //TRACE_COMP(MBAP),
  TRACE_COMP(MB_RTU_SLAVE),
  TRACE_COMP(MB_RTU_MASTER),
  TRACE_COMP(APP_WEB),
  TRACE_COMP(APP_MB_SLAVE),
  TRACE_COMP(APP_MB_MASTER),
  TRACE_COMP(APP_CLI),
  //TRACE_COMP(MB_TCP_MASTER),
  //TRACE_COMP(MB_MASTER),
};

static evloop_thread_t      _main_thread = 
{
  .init = main_thread_init,
  .fini = NULL,
};

void
app_init_complete_signal(void)
{
  completion_signal(&_comp);
}

void
app_init_complete_wait(void)
{
  completion_wait(&_comp);
}

static void
main_thread_init(evloop_thread_t* thrd)
{
  completion_init(&_comp);

  app_core_init();
  app_modbus_slave_init();
  app_modbus_master_init();

  app_cli_init();
  app_webserver_init();

  app_cli_go();
  app_webserver_go();
}

static void
go_daemon(void)
{
  // FIXME close all std descriptors
  // FIXME open log redirected to std descriptors
  // FIXME chdir to proper position
  // FIXME setup umask 
  daemon(TRUE, TRUE);
}

int
main(int argc, char** argv)
{
  const cmd_option* option;

  lib_utils_init();

  cmd_opt_handle(argc, argv);
  option = cmd_opt_get();

  if(option->foreground == FALSE)
  {
    go_daemon();
  }

  debug_log_init(option->foreground ? NULL : "./debug.out");
  app_config_init(NULL);

  debug_log("starting system\n");

  trace_init(_initial_trace_setup, NARRAY(_initial_trace_setup));

  evloop_default(&_main_thread);
}

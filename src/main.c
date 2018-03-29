#include <unistd.h>
#include "common_def.h"
#include "cmd_option.h"
#include "debug_log.h"
#include "cli.h"
#include "utils.h"
#include "evloop_thread.h"
#include "trace.h"

static void main_thread_init(evloop_thread_t* thrd);

static uint32_t   _initial_trace_setup[] =
{
  TRACE_COMP(MAIN),
  TRACE_COMP(TEST),
  TRACE_COMP(TASK),
  TRACE_COMP(SOCK_ERR),
  TRACE_COMP(SERIAL),
  TRACE_COMP(CLI_TELNET),
  TRACE_COMP(CLI_SERIAL),
  TRACE_COMP(CLI),
  TRACE_COMP(MB_TCP_SLAVE),
  TRACE_COMP(MBAP),
  TRACE_COMP(MB_RTU_SLAVE),
  TRACE_COMP(MB_RTU_MASTER),
  TRACE_COMP(MB_TCP_MASTER),
  TRACE_COMP(MB_MASTER),
};

static evloop_thread_t      _main_thread = 
{
  .init = main_thread_init,
  .fini = NULL,
};

static void
main_thread_init(evloop_thread_t* thrd)
{
  cli_init(NULL, 0);
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

  debug_log("starting system\n");

  trace_init(_initial_trace_setup, sizeof(_initial_trace_setup)/sizeof(uint32_t));

  evloop_default(&_main_thread);
}

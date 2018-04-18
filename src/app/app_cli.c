#include <stdio.h>
#include "app_cli.h"
#include "app_config.h"
#include "cli.h"
#include "trace.h"
#include "app_init_completion.h"
#include "evloop_thread.h"
#include "completion.h"
#include "app_modbus_slave.h"
#include "app_modbus_master.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
static void app_cli_thread_init(evloop_thread_t* thrd);
static void app_cli_thread_fini(evloop_thread_t* thrd);

static void cli_command_modbus(cli_intf_t* intf, int argc, const char** argv);

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

static cli_command_t    _app_commands[] =
{
  {
    "modbus",
    "modbus related command",
    cli_command_modbus,
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// command implementation
//
////////////////////////////////////////////////////////////////////////////////
static void
cli_command_modbus(cli_intf_t* intf, int argc, const char** argv)
{
  cJSON* ret = NULL;

  if(argc < 2)
  {
    goto command_error;
  }

  if(strcmp(argv[1], "slave") == 0)
  {
    cJSON*    slave_list;

    ret = app_api_modbus_slave_get_stat();
    if(ret == NULL)
    {
      cli_printf(intf, "internal error!!!"CLI_EOL CLI_EOL);
      return;
    }

    slave_list = cJSON_GetObjectItem(ret, "slave_list");

    cli_printf(intf, "Number of slaves: %d"CLI_EOL, cJSON_GetArraySize(slave_list));

    for(int i = 0; i < cJSON_GetArraySize(slave_list); i++)
    {
      cJSON* slave;

      slave = cJSON_GetArrayItem(slave_list, i);

      cli_printf(intf, "=========== slave : %d ============="CLI_EOL, i);
      cli_printf(intf, "slave type: %s"CLI_EOL, cJSON_GetObjectItem(slave, "slave_type")->valuestring);
      cli_printf(intf, "address:    %d"CLI_EOL, cJSON_GetObjectItem(slave, "slave_address")->valueint);
      cli_printf(intf, "rx frames:  %d"CLI_EOL, cJSON_GetObjectItem(slave, "rx_frames")->valueint);
      cli_printf(intf, "tx frames:  %d"CLI_EOL, cJSON_GetObjectItem(slave, "tx_frames")->valueint);
      cli_printf(intf, "my frames:  %d"CLI_EOL, cJSON_GetObjectItem(slave, "my_frames")->valueint);
      cli_printf(intf, "req fails:  %d"CLI_EOL, cJSON_GetObjectItem(slave, "req_fails")->valueint);
      cli_printf(intf, "crc error:  %d"CLI_EOL, cJSON_GetObjectItem(slave, "rx_crc_error")->valueint);
      cli_printf(intf, "overflow:   %d"CLI_EOL, cJSON_GetObjectItem(slave, "rx_buffer_overflow")->valueint);

      if(strcmp(cJSON_GetObjectItem(slave, "slave_type")->valuestring, "tcp") == 0)
      {
        cJSON* connections;

        cli_printf(intf, "bound to:   %s"CLI_EOL, cJSON_GetObjectItem(slave, "bound_address")->valuestring);

        connections = cJSON_GetObjectItem(slave, "connections");
        for(int j = 0; j < cJSON_GetArraySize(connections); j++)
        {
          cJSON* conn;

          conn = cJSON_GetArrayItem(connections, j);
          cli_printf(intf, "Connection: %s:%d"CLI_EOL, 
              cJSON_GetObjectItem(conn, "ip")->valuestring,
              cJSON_GetObjectItem(conn, "port")->valueint);
        }
      }
      else
      {
      }
    }
    cJSON_Delete(ret);
  }
  else if(strcmp(argv[1], "master") == 0)
  {
    cJSON*    master_list;

    ret = app_api_modbus_master_get_stat();
    if(ret == NULL)
    {
      cli_printf(intf, "internal error!!!"CLI_EOL CLI_EOL);
      return;
    }

    master_list = cJSON_GetObjectItem(ret, "master_list");

    cli_printf(intf, "Number of masters: %d"CLI_EOL, cJSON_GetArraySize(master_list));

    for(int i = 0; i < cJSON_GetArraySize(master_list); i++)
    {
      cJSON* master;

      master = cJSON_GetArrayItem(master_list, i);

      cli_printf(intf, "=========== master : %d ============="CLI_EOL, i);
      cli_printf(intf, "master type: %s"CLI_EOL, cJSON_GetObjectItem(master, "master_type")->valuestring);

      if(strcmp(cJSON_GetObjectItem(master, "master_type")->valuestring, "tcp") == 0)
      {
        cli_printf(intf, "server ip:   %s"CLI_EOL, cJSON_GetObjectItem(master, "server_ip")->valuestring);
        cli_printf(intf, "server port: %d"CLI_EOL, cJSON_GetObjectItem(master, "server_port")->valueint);
        cli_printf(intf, "connected:   %s"CLI_EOL, cJSON_GetObjectItem(master, "connected")->type == cJSON_True ? "yes" : "no");
      }
      else
      {
      }
    }
    cJSON_Delete(ret);
  }
  else
  {
    goto command_error;
  }

  return;

command_error:
  cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
  cli_printf(intf, "%s slave|master"CLI_EOL, argv[0]);
}

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
  cli_init(&cfg, _app_commands, NARRAY(_app_commands));

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

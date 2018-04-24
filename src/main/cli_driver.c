#include <stdio.h>
#include <stdlib.h>
#include "cli_driver.h"
#include "cfg_mgr.h"
#include "cli.h"
#include "trace.h"
#include "app_init_completion.h"
#include "evloop_thread.h"
#include "completion.h"
#include "modbus_slave_driver.h"
#include "modbus_master_driver.h"
#include "channel_manager.h"
#include "alarm_manager.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
static void cli_driver_thread_init(evloop_thread_t* thrd);
static void cli_driver_thread_fini(evloop_thread_t* thrd);

static void cli_command_modbus(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_channel(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_alarm(cli_intf_t* intf, int argc, const char** argv);

////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
////////////////////////////////////////////////////////////////////////////////
static evloop_thread_t    _app_cli_thread =
{
  .init = cli_driver_thread_init,
  .fini = cli_driver_thread_fini,
};

static completion_t     _go_signal;

static cli_command_t    _app_commands[] =
{
  {
    "modbus",
    "modbus related commands",
    cli_command_modbus,
  },
  {
    "channel",
    "channel related commands",
    cli_command_channel,
  },
  {
    "alarm",
    "alarm related commands",
    cli_command_alarm,
  },
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

    ret = modbus_slave_driver_get_stat();
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

    ret = modbus_master_driver_get_stat();
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

static void
cli_command_channel(cli_intf_t* intf, int argc, const char** argv)
{
  if(argc < 2)
  {
    goto command_error;
  }

  if(strcmp(argv[1], "status") == 0)
  {
    channel_status_t    status;
    uint32_t            chnl_num;

    if(argc != 3)
    {
      goto command_error;
    }

    chnl_num = atoi(argv[2]);

    if(channel_manager_get_channel_stat(chnl_num, &status) == -1)
    {
      cli_printf(intf, "can't find channel number %d"CLI_EOL, chnl_num);
      return;
    }

    cli_printf(intf, "channel status for channel #%d"CLI_EOL, chnl_num);
    cli_printf(intf, "raw value:    %d"CLI_EOL, status.raw_val);

    if(status.chnl_type == channel_type_digital)
    {
      cli_printf(intf, "eng value:    %s"CLI_EOL, status.eng_val.b ? "on" : "off");
    }
    else
    {
      cli_printf(intf, "eng value:    %.2f"CLI_EOL, status.eng_val.f);
    }
    cli_printf(intf, "sensor fault: %s"CLI_EOL, status.sensor_fault ? "fault" : "normal");
  }
  else
  {
    goto command_error;
  }

  return;

command_error:
  cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
  cli_printf(intf, "%s status chnl-num"CLI_EOL, argv[0]);
}

static void
cli_command_alarm(cli_intf_t* intf, int argc, const char** argv)
{
  uint32_t          alarm_num;

  if(argc < 2)
  {
    goto command_error;
  }

  if(strcmp(argv[1], "status") == 0)
  {
    alarm_status_t    status;

    if(argc != 3)
    {
      goto command_error;
    }

    alarm_num = atoi(argv[2]);

    if(alarm_manager_get_alarm_status(alarm_num, &status) == -1)
    {
      cli_printf(intf, "can't find alarm number %d"CLI_EOL, alarm_num);
      return;
    }

    cli_printf(intf, "alarm status for alarm #%d"CLI_EOL, alarm_num);
    cli_printf(intf, "alarm state: %s"CLI_EOL, alarm_get_string_state(status.state));
  }
  else if(strcmp(argv[1], "ack") == 0)
  {
    if(argc != 3)
    {
      goto command_error;
    }

    alarm_num = atoi(argv[2]);
    cli_printf(intf, "acking alarm #%d"CLI_EOL, alarm_num);
    alarm_manager_ack_alarm(alarm_num);
  }
  else if(strcmp(argv[1], "config_dig") == 0)
  {
    // alarm config_dig #num [on|off] delay

    alarm_runtime_config_t  cfg;

    if(argc != 5)
    {
      goto command_error;
    }

    alarm_num = atoi(argv[2]);

    if(strcmp(argv[3], "on") == 0)
    {
      cfg.set_point.b = TRUE;
    }
    else if(strcmp(argv[3], "off") == 0)
    {
      cfg.set_point.b = FALSE;
    }
    else
    {
      goto command_error;
    }

    cfg.delay = atoi(argv[4]);
    cfg_mgr_update_alarm_cfg(alarm_num, &cfg);
    cli_printf(intf, "done updating alarm %d"CLI_EOL, alarm_num);
  }
  else if(strcmp(argv[1], "config_ana") == 0)
  {
    // alarm config_dig #num set_point delay
  }
  else
  {
    goto command_error;
  }

  return;

command_error:
  cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
  cli_printf(intf, "%s status alarm-num"CLI_EOL, argv[0]);
  cli_printf(intf, "%s ack alarm-num"CLI_EOL, argv[0]);
  cli_printf(intf, "%s config_dig alarm-num [on|off] delay"CLI_EOL, argv[0]);
  cli_printf(intf, "%s config_ana alarm-num set_point delay"CLI_EOL, argv[0]);
}

////////////////////////////////////////////////////////////////////////////////
//
// thread initializer
//
////////////////////////////////////////////////////////////////////////////////
static void
cli_driver_thread_init(evloop_thread_t* thrd)
{
  cli_config_t    cfg;

  TRACE(CLI_DRIVER, "initializing CLI\n");

  cfg_mgr_get_cli_config(&cfg);
  cli_init(&cfg, _app_commands, NARRAY(_app_commands));

  TRACE(CLI_DRIVER, "done initializing CLI\n");

  app_init_complete_signal();

  TRACE(CLI_DRIVER, "waiting for go signal from main\n");
  completion_wait(&_go_signal);
  TRACE(CLI_DRIVER, "got go signal from main. entering main loop\n");
}

static void
cli_driver_thread_fini(evloop_thread_t* thrd)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////

void
cli_driver_init(void)
{
  TRACE(CLI_DRIVER, "starting CLI\n");

  completion_init(&_go_signal);

  evloop_thread_init(&_app_cli_thread);
  evloop_thread_run(&_app_cli_thread);

  app_init_complete_wait();
}

void
cli_driver_go(void)
{
  completion_signal(&_go_signal);
}

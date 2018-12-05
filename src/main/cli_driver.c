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
#include "math_util.h"
#include "core_driver.h"
#include "logger.h"
#include "logger_sql3_common.h"

#include "app.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
static void cli_driver_thread_init(evloop_thread_t* thrd);
static void cli_driver_thread_fini(evloop_thread_t* thrd);

static void cli_command_core(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_modbus(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_channel(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_alarm(cli_intf_t* intf, int argc, const char** argv);
static void cli_command_signal_trace(cli_intf_t* intf, int argc, const char** argv);

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
    "core",
    "core related commands",
    cli_command_core,
  },
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
  {
    "signal_trace",
    "channel signal tracing related commands",
    cli_command_signal_trace,
  }
};

////////////////////////////////////////////////////////////////////////////////
//
// command implementation
//
////////////////////////////////////////////////////////////////////////////////
static void
cli_command_core(cli_intf_t* intf, int argc, const char** argv)
{
  if(argc < 2)
  {
    goto command_error;
  }

  if(strcmp(argv[1], "status") == 0)
  {
    core_driver_stat_t    stat;

    core_driver_get_stat(&stat);

    cli_printf(intf, "input scan min:  %d ms"CLI_EOL, stat.input_scan_min);
    cli_printf(intf, "input scan max:  %d ms"CLI_EOL, stat.input_scan_max);
    cli_printf(intf, "input scan avg:  %d ms"CLI_EOL, stat.input_scan_avg);

    cli_printf(intf, "output scan min: %d ms"CLI_EOL, stat.output_scan_min);
    cli_printf(intf, "output scan max: %d ms"CLI_EOL, stat.output_scan_max);
    cli_printf(intf, "output scan avg: %d ms"CLI_EOL, stat.output_scan_avg);

    cli_printf(intf, "app min:         %d ms"CLI_EOL, stat.app_min);
    cli_printf(intf, "app max:         %d ms"CLI_EOL, stat.app_max);
    cli_printf(intf, "app avg:         %d ms"CLI_EOL, stat.app_avg);
  }
  else if(strcmp(argv[1], "app_debug") == 0)
  {
    app_debug();
  }
  else 
  {
    goto command_error;
  }

  return;

command_error:
  cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
  cli_printf(intf, "%s status|app_debug"CLI_EOL, argv[0]);
}

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
        cli_printf(intf, "server ip:      %s"CLI_EOL, cJSON_GetObjectItem(master, "server_ip")->valuestring);
        cli_printf(intf, "server port:    %d"CLI_EOL, cJSON_GetObjectItem(master, "server_port")->valueint);
        cli_printf(intf, "connected:      %s"CLI_EOL, cJSON_GetObjectItem(master, "connected")->type == cJSON_True ? "yes" : "no");
        cli_printf(intf, "n_conn_attempt: %d"CLI_EOL, cJSON_GetObjectItem(master, "n_conn_attempt")->valueint);
        cli_printf(intf, "n_conn_success: %d"CLI_EOL, cJSON_GetObjectItem(master, "n_conn_success")->valueint);
        cli_printf(intf, "n_conn_fail:    %d"CLI_EOL, cJSON_GetObjectItem(master, "n_conn_fail")->valueint);
        cli_printf(intf, "n_disconnect:   %d"CLI_EOL, cJSON_GetObjectItem(master, "n_disconnect")->valueint);
      }
      else
      {
      }
      cli_printf(intf, "n_request:      %d"CLI_EOL, cJSON_GetObjectItem(master, "n_request")->valueint);
      cli_printf(intf, "n_response:     %d"CLI_EOL, cJSON_GetObjectItem(master, "n_response")->valueint);
      cli_printf(intf, "n_rsp_timeout:  %d"CLI_EOL, cJSON_GetObjectItem(master, "n_rsp_timeout")->valueint);
      cli_printf(intf, "min time:       %dms"CLI_EOL, cJSON_GetObjectItem(master, "min_transaction_time")->valueint);
      cli_printf(intf, "max time:       %dms"CLI_EOL, cJSON_GetObjectItem(master, "max_transaction_time")->valueint);
      cli_printf(intf, "avg time:       %dms"CLI_EOL, cJSON_GetObjectItem(master, "avg_time")->valueint);
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
__print_out_channel_status(cli_intf_t* intf, uint32_t chnl_num, channel_status_t* status)
{
  cli_printf(intf, "channel status for channel #%d"CLI_EOL, chnl_num);
  cli_printf(intf, "raw value:    %d"CLI_EOL, status->raw_val);

  if(status->chnl_type == channel_type_digital)
  {
    cli_printf(intf, "eng value:    %s"CLI_EOL, status->eng_val.b ? "on" : "off");
  }
  else
  {
    cli_printf(intf, "eng value:    %.2f"CLI_EOL, status->eng_val.f);
    cli_printf(intf, "raw sensor:   %.2f"CLI_EOL, status->raw_sensor_val);
  }
  cli_printf(intf, "sensor fault: %s"CLI_EOL, status->sensor_fault ? "fault" : "normal");
}

static void
cli_command_channel(cli_intf_t* intf, int argc, const char** argv)
{
  uint32_t            chnl_num;

  if(argc < 2)
  {
    goto command_error;
  }

  if(strcmp(argv[1], "status") == 0)
  {
    channel_status_t    status;

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
    __print_out_channel_status(intf, chnl_num, &status);
  }
  else if(strcmp(argv[1], "status_ranged") == 0)
  {
    uint32_t    end_chnl;
    int         ndx;
    channel_status_t    status;

    if(argc != 4)
    {
      goto command_error;
    }

    chnl_num = atoi(argv[2]);
    end_chnl = atoi(argv[3]);

    ndx = cfg_mgr_get_channel_index_equal_or_bigger(chnl_num);
    while(ndx != -1)
    {
      chnl_num = cfg_mgr_get_channel_from_index(ndx);
      if(chnl_num > end_chnl)
      {
        break;
      }
      channel_manager_get_channel_stat(chnl_num, &status);
      __print_out_channel_status(intf, chnl_num, &status);

      ndx = cfg_mgr_get_channel_next(ndx);
    }
  }
  else if(strcmp(argv[1], "config_dig") == 0)
  {
    // channel config_dig #num <init>[on|off] <fail>[on|off]
    channel_runtime_config_t    cfg;

    if(argc != 5)
    {
      goto command_error;
    }

    chnl_num = atoi(argv[2]);

    if(strcmp(argv[3], "on") == 0)
    {
      cfg.init_value.b = TRUE;
    }
    else if(strcmp(argv[4], "off") == 0)
    {
      cfg.init_value.b = FALSE;
    }
    else
    {
      goto command_error;
    }

    if(strcmp(argv[4], "on") == 0)
    {
      cfg.failsafe_value.b = TRUE;
    }
    else if(strcmp(argv[4], "off") == 0)
    {
      cfg.failsafe_value.b = FALSE;
    }
    else
    {
      goto command_error;
    }

    cfg_mgr_update_channel_cfg(chnl_num, &cfg);
    cli_printf(intf, "done updating channel %d"CLI_EOL, chnl_num);
  }
  else if(strcmp(argv[1], "config_ana") == 0)
  {
    // channel config_dig #num <init> <fail> <min> <max>
    channel_runtime_config_t    cfg;

    if(argc != 7)
    {
      goto command_error;
    }

    chnl_num = atoi(argv[2]);

    cfg.init_value.f      = atod_round_off(argv[3], 2);
    cfg.failsafe_value.f  = atod_round_off(argv[4], 2);
    cfg.min_val           = atod_round_off(argv[5], 2);
    cfg.max_val           = atod_round_off(argv[6], 2);

    cfg_mgr_update_channel_cfg(chnl_num, &cfg);
    cli_printf(intf, "done updating channel %d"CLI_EOL, chnl_num);
  }
  else if(strcmp(argv[1], "config_ltable") == 0)
  {
    // channel config_ltable #num v1 v2 v1 v2 v1 v2 ...
    lookup_table_item_t*  entries;
    lookup_table_t*       lut;
    int                   num_entries;

    if(argc < 3)
    {
      goto command_error;
    }

    chnl_num = atoi(argv[2]);

    if(argc == 3)
    {
      // delete lookup table
      cfg_mgr_update_lookup_table(chnl_num, NULL);
      cli_printf(intf, "done cleaning lookup table for  channel %d"CLI_EOL, chnl_num);
      return;
    }

    if((argc -3) < 4 || ((argc -3) % 2) != 0)
    {
      // at least 2 entries and even number of arguments
      goto command_error;
    }

    num_entries = (argc - 3) / 2;

    lut = malloc(sizeof(lookup_table_t));
    entries = malloc(sizeof(lookup_table_item_t) * num_entries);
    for(int i = 0; i < num_entries; i++ )
    {
      entries[i].v1 = atod_round_off(argv[3 + i * 2], 2);
      entries[i].v2 = atod_round_off(argv[3 + i * 2 + 1], 2);
    }

    lookup_table_init(lut, entries, num_entries);
    cfg_mgr_update_lookup_table(chnl_num, lut);

    free(entries);
  }
  else
  {
    goto command_error;
  }

  return;

command_error:
  cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
  cli_printf(intf, "%s status chnl-num"CLI_EOL, argv[0]);
  cli_printf(intf, "%s status_ranged start end"CLI_EOL, argv[0]);
  cli_printf(intf, "%s config_dig chnl-num init(on|off) fail(on|off)"CLI_EOL, argv[0]);
  cli_printf(intf, "%s config_ana chnl-num init fail min max"CLI_EOL, argv[0]);
  cli_printf(intf, "%s config_ltable chnl-num v1 v2 v1 v2 ..."CLI_EOL, argv[0]);
}

static void
__print_out_alarm_status(cli_intf_t* intf, uint32_t alarm_num, alarm_status_t* status)
{
  cli_printf(intf, "alarm status for alarm #%d"CLI_EOL, alarm_num);
  cli_printf(intf, "alarm state: %s"CLI_EOL, alarm_get_string_state(status->state));
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

    __print_out_alarm_status(intf, alarm_num, &status);
  }
  else if(strcmp(argv[1], "status_ranged") == 0)
  {
    alarm_status_t    status;
    uint32_t          end_alarm;
    int               ndx;

    if(argc != 4)
    {
      goto command_error;
    }

    alarm_num = atoi(argv[2]);
    end_alarm = atoi(argv[3]);

    ndx = cfg_mgr_get_alarm_index_equal_or_bigger(alarm_num);
    while(ndx != -1)
    {
      alarm_num = cfg_mgr_get_alarm_from_index(ndx);
      if(alarm_num > end_alarm)
      {
        break;
      }

      alarm_manager_get_alarm_status(alarm_num, &status);
      __print_out_alarm_status(intf, alarm_num, &status);
      ndx = cfg_mgr_get_alarm_next(ndx);
    }
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
    alarm_runtime_config_t  cfg;

    if(argc != 5)
    {
      goto command_error;
    }

    alarm_num = atoi(argv[2]);

    cfg.set_point.f = atod_round_off(argv[3], 2);

    cfg.delay = atoi(argv[4]);
    cfg_mgr_update_alarm_cfg(alarm_num, &cfg);
    cli_printf(intf, "done updating alarm %d"CLI_EOL, alarm_num);
  }
  else
  {
    goto command_error;
  }

  return;

command_error:
  cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
  cli_printf(intf, "%s status alarm-num"CLI_EOL, argv[0]);
  cli_printf(intf, "%s status_ranged start end"CLI_EOL, argv[0]);
  cli_printf(intf, "%s ack alarm-num"CLI_EOL, argv[0]);
  cli_printf(intf, "%s config_dig alarm-num [on|off] delay"CLI_EOL, argv[0]);
  cli_printf(intf, "%s config_ana alarm-num set_point delay"CLI_EOL, argv[0]);
}

static int
__get_all_trace_channels_cb(void* data, int argc, char** argv, char** azColName)
{
  cli_intf_t* intf = (cli_intf_t*)data;

  cli_printf(intf, "CH - %s"CLI_EOL, argv[0]);

  return 0;
}

static void
cli_command_signal_trace(cli_intf_t* intf, int argc, const char** argv)
{
  if(argc < 2) goto command_error;


  if(strcmp(argv[1], "set") == 0)
  {
    uint32_t*   chnls = NULL;
    uint32_t    n_chnls;

    n_chnls = argc - 2;

    if(n_chnls > 0)
    {
      chnls = malloc(sizeof(uint32_t) * n_chnls);
      if(chnls == NULL)
      {
        cli_printf(intf, "failed to malloc"CLI_EOL);
        return;
      }

      for(uint32_t i = 0; i < n_chnls; i++)
      {
        chnls[i] = atoi(argv[2 + i]);
        cli_printf(intf, "CH - %d"CLI_EOL, chnls[i]);
      }
    }

    cli_printf(intf, "setting channel trace for %d channels"CLI_EOL, n_chnls);

    logger_signal_trace_set_chnls(chnls, n_chnls);

    if(chnls != NULL)
    {
      free(chnls);
    }
  }
  else if(strcmp(argv[1], "show") == 0)
  {
    sqlite3* db;

    db = logger_db_open(LOGGER_DB);

    if(db == NULL)
    {
      cli_printf(intf, "failed to open logger db"CLI_EOL);
      return ;
    }

    if(logger_db_get_all_trace_channels(db, __get_all_trace_channels_cb, intf) == FALSE)
    {
      cli_printf(intf, "failed to retrive trace channels: %s"CLI_EOL, sqlite3_errmsg(db));
    }
    else
    {
      cli_printf(intf, CLI_EOL);
    }
    logger_db_close(db);
  }
  else
  {
    goto command_error;
  }

  return;

command_error:
  cli_printf(intf, "invalid argument!!!"CLI_EOL CLI_EOL);
  cli_printf(intf, "%s set <chnls...> "CLI_EOL, argv[0]);
  cli_printf(intf, "%s show"CLI_EOL, argv[0]);
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

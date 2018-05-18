#include <stdlib.h>
#include "core_driver.h"
#include "cfg_mgr.h"
#include "evloop_thread.h"
#include "evloop_timer.h"
#include "channel_manager.h"
#include "alarm_manager.h"
#include "app_init_completion.h"
#include "trace.h"
#include "time_util.h"

static void app_core_thread_init(evloop_thread_t* thrd);
static void app_core_thread_fini(evloop_thread_t* thrd);

static double             _loop_interval;

static core_driver_stat_t _stat;

static evloop_timer_t     _update_timer;
static evloop_timer_t     _trace_timer;

static evloop_thread_t    _app_core_thread = 
{
  .init = app_core_thread_init,
  .fini = app_core_thread_fini,
};

static double _log_interval;

static void
app_core_run_application(void)
{
  //
  // FIXME run control application
  //
}

static void
app_core_udpate(evloop_timer_t* t, void* arg)
{
  unsigned long   start,
                  took;

  //static uint32_t   cnt = 0;

  //TRACE(CORE_DRIVER, "updating... %d\n", cnt++);

  start = time_util_get_sys_clock_in_ms();
  channel_manager_update_input();
  took = time_util_get_sys_clock_elapsed_in_ms(start);

  _stat.input_scan_min = MIN(_stat.input_scan_min, took);
  _stat.input_scan_max = MAX(_stat.input_scan_max, took);
  _stat.input_scan_avg = (_stat.input_scan_avg + took) / 2;

  start = time_util_get_sys_clock_in_ms();
  app_core_run_application();
  took = time_util_get_sys_clock_elapsed_in_ms(start);

  _stat.app_min = MIN(_stat.app_min, took);
  _stat.app_max = MAX(_stat.app_max, took);
  _stat.app_avg = (_stat.app_avg + took) / 2;

  start = time_util_get_sys_clock_in_ms();
  channel_manager_update_output();
  took = time_util_get_sys_clock_elapsed_in_ms(start);

  _stat.output_scan_min = MIN(_stat.output_scan_min, took);
  _stat.output_scan_max = MAX(_stat.output_scan_max, took);
  _stat.output_scan_avg = (_stat.output_scan_avg + took) / 2;

  evloop_timer_start(&_update_timer, _loop_interval, 0);
}

static void
app_core_do_log(evloop_timer_t* t, void* arg)
{
  channel_manager_do_log();
}

static void
app_core_thread_init(evloop_thread_t* thrd)
{
  TRACE(CORE_DRIVER, "initializing app_core before thread loop\n");

  evloop_timer_init(&_update_timer, app_core_udpate, NULL);
  evloop_timer_start(&_update_timer, _loop_interval, 0);

  evloop_timer_init(&_trace_timer, app_core_do_log, NULL);
  evloop_timer_start(&_trace_timer, _log_interval, _log_interval);

  TRACE(CORE_DRIVER, "done initializing app_core\n");

  app_init_complete_signal();
}

static void
app_core_thread_fini(evloop_thread_t* thrd)
{
  TRACE(CORE_DRIVER, "shutting down app_core thread\n");
}

static lookup_table_t*
__load_lookup_table(int chnl_ndx)
{
  int                     num_entries;
  double                  raw,
                          eng;
  lookup_table_t*         lut;
  lookup_table_item_t*    items;

  num_entries = cfg_mgr_get_num_lookup_table_of_channel_at(chnl_ndx);
  if(num_entries == 0)
  {
    return NULL;
  }

  items = malloc(sizeof(lookup_table_item_t) * num_entries);
  lut   = malloc(sizeof(lookup_table_t));

  for(int i = 0; i < num_entries; i++)
  {
    cfg_mgr_get_lookup_table_of_channel_at(chnl_ndx, i, &raw, &eng);

    items[i].v1   = raw;
    items[i].v2   = eng;
  }

  lookup_table_init(lut, items, num_entries);

  free(items);

  return lut;
}

static void
__load_channels(void)
{
  int                             num_channels;
  core_driver_channel_config_t    chnl_cfg;
  channel_t*                      chnl;
  unsigned long                   start_time;

  start_time = time_util_get_sys_clock_in_ms();

  TRACE(CORE_DRIVER, "loading channels\n");
  num_channels = cfg_mgr_get_num_channels();

  for(int i = 0; i < num_channels; i++)
  {
    cfg_mgr_get_channel_at(i, &chnl_cfg);
    chnl = channel_alloc(chnl_cfg.chnl_num, chnl_cfg.chnl_type, chnl_cfg.chnl_dir);

    chnl->failsafe_value    = chnl_cfg.failsafe_val;
    chnl->init_value        = chnl_cfg.init_val;
    chnl->eng_value         = chnl_cfg.init_val;

    if(chnl_cfg.chnl_type == channel_type_analog)
    {
      chnl->min_val       = chnl_cfg.min_val;
      chnl->max_val       = chnl_cfg.max_val;

      chnl->lookup_table  = __load_lookup_table(i);
    }
    channel_manager_add_channel(chnl);
  }
  TRACE(CORE_DRIVER, "done loading %d channels, took %d ms\n",
      num_channels,
      time_util_get_sys_clock_elapsed_in_ms(start_time));
}

static void
__load_alarms(void)
{
  int                         num_alarms;
  core_driver_alarm_config_t  alm_cfg;
  alarm_t*                    alarm;
  unsigned long                   start_time;

  start_time = time_util_get_sys_clock_in_ms();
  TRACE(CORE_DRIVER, "loading alarms\n");
  num_alarms = cfg_mgr_get_num_alarms();

  for(int i = 0; i < num_alarms; i++)
  {
    cfg_mgr_get_alarm_at(i, &alm_cfg);

    alarm = alarm_alloc(alm_cfg.alarm_num, alm_cfg.chnl_num,
        alm_cfg.severity, alm_cfg.trigger_type,
        alm_cfg.set_point,
        alm_cfg.delay);

    alarm_manager_add_alarm(alarm);
  }
  TRACE(CORE_DRIVER, "done loading %d alarms, took %d ms\n",
      num_alarms,
      time_util_get_sys_clock_elapsed_in_ms(start_time));
}

void
core_driver_init(void)
{
  core_driver_config_t    cfg;
  logger_config_t         logging_cfg;

  TRACE(CORE_DRIVER, "starting up core driver\n");

  cfg_mgr_get_core_driver_config(&cfg);
  _loop_interval = cfg.loop_interval / 1000.0;

  cfg_mgr_get_logging_config(&logging_cfg);

  TRACE(CORE_DRIVER, "logging interval : %u ms\n", logging_cfg.interval);

  _log_interval = logging_cfg.interval / 1000.0;


  _stat.input_scan_min  = 0xffffffff;
  _stat.input_scan_max  = 0;
  _stat.input_scan_avg  = 0;

  _stat.output_scan_min = 0xffffffff;
  _stat.output_scan_max = 0;
  _stat.output_scan_avg = 0;

  _stat.app_min = 0xffffffff;
  _stat.app_max = 0;
  _stat.app_avg = 0;

  TRACE(CORE_DRIVER, "core driver loop interval: %.3f sec\n", _loop_interval);

  evloop_thread_init(&_app_core_thread);

  channel_manager_init();
  alarm_manager_init();

  __load_channels();
  __load_alarms();

  channel_manager_init_channels();

  evloop_thread_run(&_app_core_thread);

  app_init_complete_wait();
}

void
core_driver_get_stat(core_driver_stat_t* stat)
{
  evloop_thread_lock(&_app_core_thread);

  *stat = _stat;

  evloop_thread_unlock(&_app_core_thread);
}

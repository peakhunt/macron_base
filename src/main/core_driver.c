#include <stdlib.h>
#include "core_driver.h"
#include "cfg_mgr.h"
#include "evloop_thread.h"
#include "evloop_timer.h"
#include "channel_manager.h"
#include "alarm_manager.h"
#include "app_init_completion.h"
#include "trace.h"

static void app_core_thread_init(evloop_thread_t* thrd);
static void app_core_thread_fini(evloop_thread_t* thrd);

static evloop_timer_t     _update_timer;

static evloop_thread_t    _app_core_thread = 
{
  .init = app_core_thread_init,
  .fini = app_core_thread_fini,
};

static void
app_core_udpate(evloop_timer_t* t, void* arg)
{
  //static uint32_t   cnt = 0;

  //TRACE(CORE_DRIVER, "updating... %d\n", cnt++);

  channel_manager_update_input();
  //
  // FIXME run control application
  //
  channel_manager_update_output();

  evloop_timer_start(&_update_timer, 0.010, 0);
}

static void
app_core_thread_init(evloop_thread_t* thrd)
{
  TRACE(CORE_DRIVER, "initializing app_core before thread loop\n");

  evloop_timer_init(&_update_timer, app_core_udpate, NULL);
  evloop_timer_start(&_update_timer, 0.05, 0);

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
  float                   raw,
                          eng;
  lookup_table_t*         lut;
  lookup_table_item_t*    items;

  num_entries = cfg_mgr_get_num_lookup_table_of_channel_at(chnl_ndx);
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

  TRACE(CORE_DRIVER, "loading channels\n");
  num_channels = cfg_mgr_get_num_channels();

  for(int i = 0; i < num_channels; i++)
  {
    cfg_mgr_get_channel_at(i, &chnl_cfg);
    chnl = channel_alloc(chnl_cfg.chnl_num, chnl_cfg.chnl_type, chnl_cfg.chnl_dir);

    if(chnl_cfg.chnl_type == channel_type_analog)
    {
      chnl->lookup_table = __load_lookup_table(i);
    }

    chnl->failsafe_value    = chnl_cfg.failsafe_val;
    chnl->init_value        = chnl_cfg.init_val;
    chnl->eng_value         = chnl_cfg.init_val;

    channel_manager_add_channel(chnl);
  }
  TRACE(CORE_DRIVER, "done loading %d channels\n", num_channels);
}

static void
__load_alarms(void)
{
  int                         num_alarms;
  core_driver_alarm_config_t  alm_cfg;
  alarm_t*                    alarm;

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
  TRACE(CORE_DRIVER, "done loading %d alarms\n", num_alarms);
}

void
core_driver_init(void)
{
  TRACE(CORE_DRIVER, "starting up core driver\n");

  evloop_thread_init(&_app_core_thread);

  channel_manager_init();
  alarm_manager_init();

  __load_channels();
  __load_alarms();

  evloop_thread_run(&_app_core_thread);

  app_init_complete_wait();
}

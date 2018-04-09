#include "app_core.h"
#include "app_config.h"
#include "evloop_thread.h"
#include "evloop_timer.h"
#include "channel_manager.h"
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

  //TRACE(APP_CORE, "updating... %d\n", cnt++);

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
  TRACE(APP_CORE, "initializing app_core before thread loop\n");

  evloop_timer_init(&_update_timer, app_core_udpate, NULL);
  evloop_timer_start(&_update_timer, 0.05, 0);
}

static void
app_core_thread_fini(evloop_thread_t* thrd)
{
  TRACE(APP_CORE, "shutting down app_core thread\n");
}

static void
__load_channels(void)
{
  int num_channels;
  app_channel_config_t    chnl_cfg;
  channel_t*              chnl;

  TRACE(APP_CORE, "loading channels\n");
  num_channels = app_config_get_num_channels();

  for(int i = 0; i < num_channels; i++)
  {
    app_config_get_channel_at(i, &chnl_cfg);

    chnl = channel_alloc(chnl_cfg.chnl_num, chnl_cfg.chnl_type, chnl_cfg.chnl_dir);
    channel_manager_add_channel(chnl);
  }
  TRACE(APP_CORE, "done loading %d channels\n", num_channels);
}

void
app_core_init(void)
{
  TRACE(APP_CORE, "starting up app core\n");

  evloop_thread_init(&_app_core_thread);

  channel_manager_init();

  __load_channels();

  evloop_thread_run(&_app_core_thread);
}

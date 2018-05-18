#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <errno.h>

#include "thread.h"
#include "evloop_thread.h"
#include "thread_queue.h"
#include "time_util.h"
#include "trace.h"
#include "app_init_completion.h"
#include "completion.h"
#include "channel_manager.h"
#include "cfg_mgr.h"
#include "logger.h"
#include "logger_sql3_common.h"
#include "evloop_timer.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LOGGER_REQUEST_SIZE         1024

#define HOUR_TO_MSEC(h)             (h*60*60*1000)
#define DAYS_TO_MSEC(d)             HOUR_TO_MSEC(d*24)

struct __logger_request_t;
typedef struct __logger_request_t logger_request_t;

typedef void (*logger_request_handler)(logger_request_t* lr);

struct __logger_request_t
{
  struct list_head      le;
  unsigned long         timestamp;
  union
  {
    uint32_t    chnl;
    uint32_t    alarm;
  };
  union
  {
    logger_alarm_event_t    event;
    double                  v;
  };
  logger_request_handler    handler;
  void*                     arg;
};

typedef struct
{
  completion_t      c;
  uint32_t*         chnls;
  uint32_t          n_chnls;
} trace_set_arg_t;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// private prototypes
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void logger_thread_init(evloop_thread_t* thrd);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// module privates
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static sqlite3*       _logger_db = NULL;

static evloop_thread_t        _logger_thread =
{
  .init = logger_thread_init,
};
static ev_async               _req_q_noti;
static thread_queue_t         _req_q;

static logger_request_t       _req_buffer[LOGGER_REQUEST_SIZE];
static struct list_head       _req_buffer_pool;
static pthread_mutex_t        _req_buffer_lock;

static uint32_t*              _trace_channels = NULL;
static uint32_t               _num_trace_channels = 0;

static logger_config_t        _logger_cfg;
static evloop_timer_t         _log_clean_timer;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
logger_db_init(void)
{
  _logger_db = logger_db_open(LOGGER_DB);
  if(_logger_db == NULL)
  {
    TRACE(LOGGER, "failed to open logger db %s\n", LOGGER_DB);
    exit(-1);
  }
  TRACE(LOGGER, "logger db %s opened\n", LOGGER_DB);
}

static void
load_traced_channels()
{
  uint32_t    *chnls,
              num_chnls;

  if(logger_db_get_trace_channels(_logger_db, &chnls, &num_chnls) == FALSE)
  {
    TRACE(LOGGER, "failed to load trace channels\n");
    exit(-1);
  }

  TRACE(LOGGER, "num trace channels: %d\n", num_chnls);
  for(uint32_t i = 0; i < num_chnls; i++)
  {
    TRACE(LOGGER, "trace channel: %d\n", chnls[i]);
  }

  _trace_channels     = chnls;
  _num_trace_channels = num_chnls;

  channel_manager_set_trace_channels(chnls, num_chnls);
}

static bool
check_if_channe_is_traced(uint32_t chnl_num)
{
  for(uint32_t ndx = 0; ndx < _num_trace_channels; ndx++)
  {
    if(_trace_channels[ndx] == chnl_num)
    {
      return TRUE;
    }
  }
  return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// logger request buffer pool
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
logger_req_buffer_pool_init(void)
{
  INIT_LIST_HEAD(&_req_buffer_pool);

  for (int i = 0; i < LOGGER_REQUEST_SIZE; i++) 
  {
    INIT_LIST_HEAD(&_req_buffer[i].le);
    list_add_tail(&_req_buffer[i].le, &_req_buffer_pool);
  }

  pthread_mutex_init(&_req_buffer_lock, NULL);
}

static logger_request_t*
logger_req_buffer_pool_get(void)
{
  logger_request_t*     lr = NULL;

  pthread_mutex_lock(&_req_buffer_lock);

  if(!list_empty(&_req_buffer_pool))
  {
    lr = list_first_entry(&_req_buffer_pool, logger_request_t, le);
    list_del_init(&lr->le);
  }

  pthread_mutex_unlock(&_req_buffer_lock);

  return lr;
}

static void
logger_req_buffer_pool_put(logger_request_t* lr)
{
  pthread_mutex_lock(&_req_buffer_lock);
  
  list_add_tail(&lr->le, &_req_buffer_pool);

  pthread_mutex_unlock(&_req_buffer_lock);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// logger request handler
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
logger_chnl_log_req_handler(logger_request_t* lr)
{
  if(check_if_channe_is_traced(lr->chnl) == FALSE)
  {
    // may happen, especially during clearing due to queued requests
    return;
  }

  if(logger_db_insert_chnl(_logger_db, lr->chnl, lr->timestamp, lr->v) == FALSE)
  {
    TRACE(LOGGER, "logger_db_insert_chnl failed for channel log: %s\n", sqlite3_errmsg(_logger_db));
    return;
  }
}

static void
logger_alarm_log_req_handler(logger_request_t* lr)
{
  TRACE(LOGGER, "alarm logging: %d, %lu, %d\n", lr->alarm, lr->timestamp, lr->event);

  if(logger_db_insert_alarm(_logger_db, lr->alarm, lr->timestamp, lr->event) == FALSE)
  {
    TRACE(LOGGER, "logger_db_insert_alarm failed: %s\n", sqlite3_errmsg(_logger_db));
    return;
  }
}

static void
logger_signal_trace_set_chnls_handler(logger_request_t* lr)
{
  trace_set_arg_t*    arg   = (trace_set_arg_t*)lr->arg;
  unsigned long timestamp   = time_util_get_current_time_in_ms();

  TRACE(LOGGER, "channel trace set chnls\n");

  if(logger_db_set_trace_channels(_logger_db, arg->chnls, arg->n_chnls, timestamp) == FALSE)
  {
    TRACE(LOGGER, "logger_db_set_trace_channels failed: %s\n", sqlite3_errmsg(_logger_db));
  }
  else
  {
    channel_manager_set_trace_channels(arg->chnls, arg->n_chnls);

    if(_trace_channels)
    {
      free(_trace_channels);
    }

    _trace_channels = NULL;
    _num_trace_channels = arg->n_chnls;

    if(_num_trace_channels != 0)
    {
      _trace_channels = malloc(sizeof(uint32_t) * _num_trace_channels);
      if(_trace_channels == NULL)
      {
        _num_trace_channels = 0;
        TRACE(LOGGER, "failed to malloc _trace_channels\n");
      }
      else
      {
        memcpy(_trace_channels, arg->chnls, sizeof(uint32_t) * _num_trace_channels);
      }
    }
  }

  completion_signal(&arg->c);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// periodic cleanup
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
logger_do_cleanup(evloop_timer_t* t, void* arg)
{
  unsigned long now,
                channel_log_old,
                alarm_log_old,
                start;

  start = time_util_get_sys_clock_in_ms();

  now = time_util_get_current_time_in_ms();

  channel_log_old = now - HOUR_TO_MSEC(_logger_cfg.signal_log_keep);
  alarm_log_old   = now - DAYS_TO_MSEC(_logger_cfg.alarm_log_keep);

  TRACE(LOGGER, "cleaning channel/alarm logs %lu, %lu, %lu\n", now, channel_log_old, alarm_log_old);

#if 0
  if(logger_db_clean_up_old_channel_logs(_logger_db, channel_log_old) == FALSE)
  {
    TRACE(LOGGER, "cleaning channel logs failed\n");
  }

  if(logger_db_clean_up_old_alarm_logs(_logger_db, alarm_log_old) == FALSE)
  {
    TRACE(LOGGER, "cleaning alarm logs failed\n");
  }
#else
  if(logger_db_clean_up_old_channel_alarm_logs(_logger_db, channel_log_old, alarm_log_old) == FALSE)
  {
    TRACE(LOGGER, "cleaning alarm/channel logs failed\n");
  }
#endif

  TRACE(LOGGER, "took %lu ms to clean up logs\n", time_util_get_sys_clock_elapsed_in_ms(start));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// logger thread
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
logger_thread_init(evloop_thread_t* thrd)
{
  TRACE(LOGGER, "entering logger loop\n");

  evloop_timer_init(&_log_clean_timer, logger_do_cleanup, NULL);
  evloop_timer_start(&_log_clean_timer,
      _logger_cfg.log_clean_period,
      _logger_cfg.log_clean_period);

  app_init_complete_signal();
}

static void
logger_req_q_noti_callback(EV_P_ ev_async *w, int revents)
{
  logger_request_t*       lr;
  LIST_HEAD_DECL(tmp_list);

  thread_queue_lock(&_req_q);
  list_cut_position(&tmp_list, &_req_q.q, _req_q.q.prev);
  _req_q.count = 0;
  thread_queue_unlock(&_req_q);

  while(!list_empty(&tmp_list))
  {
    lr = list_first_entry(&tmp_list, logger_request_t, le);
    list_del_init(&lr->le);

    lr->handler(lr);

    logger_req_buffer_pool_put(lr);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public initialization entry 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
logger_init(void)
{
  TRACE(LOGGER, "starting logger\n");

  cfg_mgr_get_logging_config(&_logger_cfg);

  TRACE(LOGGER, "signal log keep: %u\n", _logger_cfg.signal_log_keep);
  TRACE(LOGGER, "alarm log keep: %u\n", _logger_cfg.alarm_log_keep);
  TRACE(LOGGER, "log clean period: %u\n", _logger_cfg.log_clean_period);

  logger_db_init();
  load_traced_channels();

  thread_queue_init(&_req_q);
  evloop_thread_init(&_logger_thread);

  ev_async_init(&_req_q_noti, logger_req_q_noti_callback);
  ev_async_start(_logger_thread.ev_loop, &_req_q_noti);

  logger_req_buffer_pool_init();

  evloop_thread_run(&_logger_thread);

  app_init_complete_wait();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public api  
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
logger_signal_log(uint32_t chnl, double v, unsigned long timestamp)
{
  logger_request_t*   lr;

  lr = logger_req_buffer_pool_get();
  if(lr == NULL)
  {
    TRACE(LOGGER, "failed to get logger request for signal logging\n");
    return;
  }

  lr->timestamp   = timestamp;
  lr->chnl        = chnl;
  lr->v           = v;
  lr->handler     = logger_chnl_log_req_handler;

  thread_queue_add(&_req_q, &lr->le);
  ev_async_send(_logger_thread.ev_loop, &_req_q_noti);
}

void
logger_alarm_log(uint32_t alarm, logger_alarm_event_t evt)
{
  logger_request_t*   lr;

  lr = logger_req_buffer_pool_get();
  if(lr == NULL)
  {
    TRACE(LOGGER, "failed to get logger request for alarm logging\n");
    return;
  }

  lr->timestamp   = time_util_get_current_time_in_ms();
  lr->alarm       = alarm;
  lr->event       = evt;
  lr->handler     = logger_alarm_log_req_handler;

  thread_queue_add(&_req_q, &lr->le);
  ev_async_send(_logger_thread.ev_loop, &_req_q_noti);
}

void
logger_signal_trace_set_chnls(uint32_t* chnls, uint32_t num_chnls)
{
  logger_request_t*   lr;
  trace_set_arg_t     arg;

  lr = logger_req_buffer_pool_get();
  if(lr == NULL)
  {
    TRACE(LOGGER, "failed to get logger request for signal trace chnls set\n");
    return;
  }

  completion_init(&arg.c);

  arg.chnls     = chnls;
  arg.n_chnls   = num_chnls;

  lr->arg       = &arg;
  lr->handler   = logger_signal_trace_set_chnls_handler;

  thread_queue_add(&_req_q, &lr->le);
  ev_async_send(_logger_thread.ev_loop, &_req_q_noti);

  completion_wait(&arg.c);
  completion_deinit(&arg.c);
}

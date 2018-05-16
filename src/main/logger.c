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
#include "logger.h"
#include "logger_sql3_common.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define LOGGER_REQUEST_SIZE         1024

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
  if(logger_db_check_if_channel_traced(_logger_db, lr->chnl) == FALSE)
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
logger_signal_trace_set_req_handler(logger_request_t* lr)
{
  completion_t*         c = (completion_t*)lr->arg;

  TRACE(LOGGER, "channel trace set: %d\n", lr->chnl);
  if(logger_db_insert_trace_channel(_logger_db, lr->chnl, time_util_get_current_time_in_ms()) == TRUE)
  {
    channel_manager_set_channel_trace(lr->chnl);
  }
  else
  {
    TRACE(LOGGER, "logger_db_insert_trace_channel failed for channel trace set: %s\n", sqlite3_errmsg(_logger_db));
  }
  completion_signal(c);
}

static void
logger_signal_trace_clear_req_handler(logger_request_t* lr)
{
  completion_t*   c = (completion_t*)lr->arg;

  TRACE(LOGGER, "channel trace clear: %d\n", lr->chnl);

  // step 1. update core channel manager
  channel_manager_clear_channel_trace(lr->chnl);

  // step 3. delete existing logs
  if(logger_db_delete_trace_channel_and_data(_logger_db, lr->chnl) == FALSE)
  {
    TRACE(LOGGER, "logger_db_delete_trace_channel_and_data failed for channel trace clear: %s\n", sqlite3_errmsg(_logger_db));
  }

  completion_signal(c);
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
  logger_db_init();

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
logger_signal_log(uint32_t chnl, double v)
{
  logger_request_t*   lr;

  lr = logger_req_buffer_pool_get();
  if(lr == NULL)
  {
    TRACE(LOGGER, "failed to get logger request for signal logging\n");
    return;
  }

  lr->timestamp   = time_util_get_current_time_in_ms();
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
logger_signal_trace_set(uint32_t chnl)
{
  logger_request_t*   lr;
  completion_t        c;

  lr = logger_req_buffer_pool_get();
  if(lr == NULL)
  {
    TRACE(LOGGER, "failed to get logger request for signal trace set\n");
    return;
  }

  completion_init(&c);

  lr->chnl    = chnl;
  lr->arg     = &c;
  lr->handler = logger_signal_trace_set_req_handler;

  thread_queue_add(&_req_q, &lr->le);
  ev_async_send(_logger_thread.ev_loop, &_req_q_noti);

  completion_wait(&c);
  completion_deinit(&c);
}

void
logger_signal_trace_clear(uint32_t chnl)
{
  logger_request_t*   lr;
  completion_t        c;

  lr = logger_req_buffer_pool_get();
  if(lr == NULL)
  {
    TRACE(LOGGER, "failed to get logger request for signal trace set\n");
    return;
  }

  completion_init(&c);

  lr->chnl    = chnl;
  lr->arg     = &c;
  lr->handler = logger_signal_trace_clear_req_handler;

  thread_queue_add(&_req_q, &lr->le);
  ev_async_send(_logger_thread.ev_loop, &_req_q_noti);

  completion_wait(&c);
  completion_deinit(&c);
}

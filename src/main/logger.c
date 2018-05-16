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
#include "logger.h"

#define LOGGER_DB     "db/logging_db.sq3"

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
    struct
    {
      double f;
      bool b;
    } chnl_v;
  };
  logger_request_handler    handler;
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
  int ret;

  ret = sqlite3_open_v2(LOGGER_DB, &_logger_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_NOMUTEX, NULL);
  if(ret != SQLITE_OK)
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
  const static char*    sql_cmd_buffer = 
    "insert into channel_log (ch_num, time_stamp, eng_val_f, eng_val_b) values (?1,?2,?3,?4)";
  sqlite3_stmt*         stmt;
  const char*           sql_error;
  int                   ret;

  sqlite3_exec(_logger_db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  ret = sqlite3_prepare_v2(_logger_db, sql_cmd_buffer, strlen(sql_cmd_buffer), &stmt, &sql_error);
  if(ret != SQLITE_OK)
  {
    TRACE(LOGGER, "sqlite3_prepare_v2 failed for channel log\n");
    return;
  }

  sqlite3_bind_int(stmt, 1, lr->chnl);
  sqlite3_bind_int(stmt, 2, lr->timestamp);
  sqlite3_bind_double(stmt, 3, lr->chnl_v.f);
  sqlite3_bind_int(stmt, 4, lr->chnl_v.b);

  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    TRACE(LOGGER, "sqlite3_step failed for channel log\n");
    return;
  }

  if(sqlite3_finalize(stmt) != SQLITE_OK)
  {
    TRACE(LOGGER, "sqlite3_finalize failed for channel log\n");
    return;
  }

  sqlite3_exec(_logger_db, "END TRANSACTION", NULL, NULL, NULL);
}

static void
logger_alarm_log_req_handler(logger_request_t* lr)
{
  const static char*    sql_cmd_buffer = 
    "insert into alarm_log (alarm, time_stamp, event) values (?1, ?2, ?3)";
  sqlite3_stmt*         stmt;
  const char*           sql_error;
  int                   ret;

  TRACE(LOGGER, "alarm logging: %d, %lu, %d\n", lr->alarm, lr->timestamp, lr->event);

  sqlite3_exec(_logger_db, "BEGIN TRANSACTION", NULL, NULL, NULL);
  ret = sqlite3_prepare_v2(_logger_db, sql_cmd_buffer, strlen(sql_cmd_buffer), &stmt, &sql_error);
  if(ret != SQLITE_OK)
  {
    TRACE(LOGGER, "sqlite3_prepare_v2 failed for alarm log:\n", sqlite3_errmsg(_logger_db));
    return;
  }

  sqlite3_bind_int(stmt, 1, lr->alarm);
  sqlite3_bind_int(stmt, 2, lr->timestamp);
  sqlite3_bind_int(stmt, 3, lr->event);

  if(sqlite3_step(stmt) != SQLITE_DONE)
  {
    TRACE(LOGGER, "sqlite3_step failed for alarm log: %s\n", sqlite3_errmsg(_logger_db));
    return;
  }

  if(sqlite3_finalize(stmt) != SQLITE_OK)
  {
    TRACE(LOGGER, "sqlite3_finalize failed for alarm log: %s\n", sqlite3_errmsg(_logger_db));
    return;
  }

  sqlite3_exec(_logger_db, "END TRANSACTION", NULL, NULL, NULL);
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
logger_signal_log(uint32_t chnl, double f, bool b)
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
  lr->chnl_v.f    = f;
  lr->chnl_v.b    = b;
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

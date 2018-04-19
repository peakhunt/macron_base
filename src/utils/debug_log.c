#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ev.h>
#include "thread.h"
#include "evloop_thread.h"
#include "thread_queue.h"
#include "list.h"
#include "debug_log.h"

///////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
///////////////////////////////////////////////////////////////////////////////
#define   LOG_BUFFER_NUM_ELEM         128
#define   LOG_BUFFER_SIZE             256

typedef struct
{
  struct list_head      le;
  uint8_t               buffer[LOG_BUFFER_SIZE];
} log_buffer_t;

///////////////////////////////////////////////////////////////////////////////
//
// module privates
//
///////////////////////////////////////////////////////////////////////////////
static log_buffer_t           _log_buffers[LOG_BUFFER_NUM_ELEM];
static struct list_head       _log_buffer_pool;
static pthread_mutex_t        _log_buffer_lock;

static evloop_thread_t        _debug_log_thread;
static ev_async               _q_noti;
static thread_queue_t         _debug_log_queue;

static FILE*                  _debug_out;

///////////////////////////////////////////////////////////////////////////////
//
// log buffer pool management
//
///////////////////////////////////////////////////////////////////////////////
static void
log_buffer_pool_init(void)
{
  INIT_LIST_HEAD(&_log_buffer_pool);

  for (int i = 0; i < LOG_BUFFER_NUM_ELEM; i++) {
    INIT_LIST_HEAD(&_log_buffers[i].le);
    list_add_tail(&_log_buffers[i].le, &_log_buffer_pool);
  }

  pthread_mutex_init(&_log_buffer_lock, NULL);
}

static log_buffer_t*
log_buffer_pool_get(void)
{
  log_buffer_t*     lb = NULL;

  pthread_mutex_lock(&_log_buffer_lock);

  if(!list_empty(&_log_buffer_pool))
  {
    lb = list_first_entry(&_log_buffer_pool, log_buffer_t, le);
    list_del_init(&lb->le);
  }

  pthread_mutex_unlock(&_log_buffer_lock);

  return lb;
}

static void
log_buffer_pool_put(log_buffer_t* lb)
{
  pthread_mutex_lock(&_log_buffer_lock);

  list_add_tail(&lb->le, &_log_buffer_pool);

  pthread_mutex_unlock(&_log_buffer_lock);
}

///////////////////////////////////////////////////////////////////////////////
//
// logger thread
//
///////////////////////////////////////////////////////////////////////////////
static void
debug_log_queue_noti_callback(EV_P_ ev_async *w, int revents)
{
  log_buffer_t*             lb;
  LIST_HEAD_DECL(tmp_list);

  thread_queue_lock(&_debug_log_queue);
  list_cut_position(&tmp_list, &_debug_log_queue.q, _debug_log_queue.q.prev);
  _debug_log_queue.count = 0;
  thread_queue_unlock(&_debug_log_queue);

  // now run it
  while(!list_empty(&tmp_list))
  {
    lb = list_first_entry(&tmp_list, log_buffer_t, le);
    list_del_init(&lb->le);

    fprintf(_debug_out, "%s", lb->buffer);
    fflush(_debug_out);

    log_buffer_pool_put(lb);
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// module publics
//
///////////////////////////////////////////////////////////////////////////////
void
debug_log_init(const char* log_file)
{
  if(log_file != NULL)
  {
    _debug_out = fopen(log_file, "a");
    if(_debug_out != NULL) 
    {
      fcntl(fileno(_debug_out), F_SETFD, FD_CLOEXEC);
    }
  }

  if(_debug_out == NULL)
  {
    _debug_out = stderr;
  }


  thread_queue_init(&_debug_log_queue);

  evloop_thread_init(&_debug_log_thread);
  ev_async_init(&_q_noti, debug_log_queue_noti_callback);
  ev_async_start(_debug_log_thread.ev_loop, &_q_noti);

  log_buffer_pool_init();
  evloop_thread_run(&_debug_log_thread);
}

void
debug_log(const char* fmt, ...)
{
  va_list               args;
  log_buffer_t*         buf;

  buf = log_buffer_pool_get();
  if(buf == NULL)
  {
    return;
  }

  va_start(args, fmt);
  vsnprintf((char*)buf->buffer, LOG_BUFFER_SIZE, fmt, args);
  va_end(args);

  thread_queue_add(&_debug_log_queue, &buf->le);
  ev_async_send(_debug_log_thread.ev_loop, &_q_noti);
}

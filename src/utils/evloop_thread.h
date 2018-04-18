#ifndef __EVLOOP_THREAD_DEF_H__
#define __EVLOOP_THREAD_DEF_H__

#include <pthread.h>
#include <ev.h>
#include "common_def.h"
#include "thread.h"

struct __evloop_thread_t;
typedef struct __evloop_thread_t evloop_thread_t;

struct __evloop_thread_t
{
  pthread_t         tid;
  struct ev_loop*   ev_loop;
  ev_async          quit_signal;
  bool              finish;
  void (*init)(evloop_thread_t* thrd);
  void (*fini)(evloop_thread_t* thrd);

  pthread_mutex_t   thread_lock;
};

extern void evloop_thread_init(evloop_thread_t* thrd);
extern void evloop_thread_run(evloop_thread_t* thrd);
extern void evloop_thread_fini(evloop_thread_t* thrd);

extern void evloop_default(evloop_thread_t* thrd);

static inline evloop_thread_t* evloop_thread_self(void)
{
  return (evloop_thread_t*)thread_getspecific();
}

static inline void
evloop_thread_lock(evloop_thread_t* thrd)
{
  pthread_mutex_lock(&thrd->thread_lock);
}

static inline void
evloop_thread_unlock(evloop_thread_t* thrd)
{
  pthread_mutex_unlock(&thrd->thread_lock);
}

#endif /* !__EVLOOP_THREAD_DEF_H__ */

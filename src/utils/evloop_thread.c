#include "evloop_thread.h"
#include "thread.h"
#include "list.h"

///////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// module privates
//
///////////////////////////////////////////////////////////////////////////////
static void*
__evloop_thread_func(void* arg)
{
  evloop_thread_t*    thrd = (evloop_thread_t*)arg;

  thread_setspecific(thrd);

  if(thrd->init)
  {
    thrd->init(thrd);
  }

  ev_ref (thrd->ev_loop);

  while(thrd->finish == FALSE)
  {
    ev_run(thrd->ev_loop, 0);
  }

  if(thrd->fini)
  {
    thrd->fini(thrd);
  }
  return NULL;
}

static void
__quit_signal_handler (EV_P_ ev_async *w, int revents)
{
  evloop_thread_t* thrd = container_of(w, evloop_thread_t, quit_signal);

  thrd->finish = TRUE;
}

///////////////////////////////////////////////////////////////////////////////
//
// module publics
//
///////////////////////////////////////////////////////////////////////////////
void
evloop_thread_init(evloop_thread_t* thrd)
{
  thrd->ev_loop = ev_loop_new(0);
  ev_async_init(&thrd->quit_signal, __quit_signal_handler);
}

void
evloop_thread_run(evloop_thread_t* thrd)
{
  ev_async_start(thrd->ev_loop, &thrd->quit_signal);
  pthread_create(&thrd->tid, NULL, __evloop_thread_func, (void*)thrd);
}

void
evloop_thread_fini(evloop_thread_t* thrd)
{
  void*   ret;

  ev_async_send(thrd->ev_loop, &thrd->quit_signal);
  pthread_join(thrd->tid, &ret);

  ev_loop_destroy(thrd->ev_loop);
}

void
evloop_default(evloop_thread_t* thrd)
{
  thrd->ev_loop = EV_DEFAULT;

  ev_async_init(&thrd->quit_signal, __quit_signal_handler);
  ev_async_start(thrd->ev_loop, &thrd->quit_signal);

  (void*)__evloop_thread_func(thrd);
}

#include "evloop_timer.h"
///////////////////////////////////////////////////////////////////////////////
//
// common ev timer callback
//
///////////////////////////////////////////////////////////////////////////////
static void
__common_ev_tmr_cb(EV_P_ ev_timer* w, int revents)
{
  evloop_timer_t*   t = container_of(w, evloop_timer_t, ev_tmr);

  t->cb(t, t->arg);
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
void
evloop_timer_init(evloop_timer_t* t, evloop_timer_cb cb, void* arg)
{
  ev_timer_init(&t->ev_tmr, __common_ev_tmr_cb, 0, 0);

  t->cb   = cb;
  t->arg  = arg;
}

void
evloop_timer_start(evloop_timer_t* t, double start, double repeat)
{
  evloop_thread_t* self = evloop_thread_self();

  ev_timer_set(&t->ev_tmr, start, repeat);
  ev_timer_start(self->ev_loop, &t->ev_tmr);
}

void
evloop_timer_stop(evloop_timer_t* t)
{
  evloop_thread_t* self = evloop_thread_self();

  ev_timer_stop(self->ev_loop, &t->ev_tmr);
}

#ifndef __EVLOOP_TIMER_DEF_H__
#define __EVLOOP_TIMER_DEF_H__

#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <ev.h>

#include "common_def.h"
#include "list.h"
#include "evloop_thread.h"

struct __evloop_timer_t;
typedef struct __evloop_timer_t evloop_timer_t;
typedef void (*evloop_timer_cb)(evloop_timer_t* t, void* arg);

struct __evloop_timer_t
{
  ev_timer          ev_tmr;
  evloop_timer_cb   cb;
  void*             arg;
};

extern void evloop_timer_init(evloop_timer_t* t, evloop_timer_cb cb, void* arg);
extern void evloop_timer_start(evloop_timer_t* t, double start, double repeat);
extern void evloop_timer_stop(evloop_timer_t* t);

static inline bool
evloop_timer_active(evloop_timer_t* t)
{
  return ev_is_active(&t->ev_tmr);
}

static inline void
evloop_timer_restart(evloop_timer_t* t, double start, double repeat)
{
  if(evloop_timer_active(t))
  {
    evloop_timer_stop(t);
  }

  evloop_timer_start(t, start, repeat);
}


#endif /* !__EVLOOP_TIMER_DEF_H__ */

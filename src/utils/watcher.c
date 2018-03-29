#include "evloop_thread.h"
#include "watcher.h"
#include "list.h"

static inline ev_io*
__get_ev_io(watcher_t* watcher, uint8_t target)
{
  if(target & WATCHER_RX_WATCH)
  {
    return &watcher->rx_io;
  }

  if(target & WATCHER_TX_WATCH)
  {
    return &watcher->tx_io;
  }

  if(target & WATCHER_ER_WATCH)
  {
    return &watcher->er_io;
  }

  //
  // BUG; let it crash
  //
  return NULL;
}

static inline void
__watcher_start(watcher_t* watcher, uint8_t target)
{
  if(watcher->flags & target)
  {
    // already started
    return;
  }

  watcher->flags |= target;

  if(watcher->flags & WATCHER_STARTED)
  {
    ev_io_start(evloop_thread_self()->ev_loop, __get_ev_io(watcher, target));
  }
}

static inline void
__watcher_stop(watcher_t* watcher, uint8_t target)
{
  if(!(watcher->flags & target))
  {
    // not even started
    return;
  }

  watcher->flags &= ~target;

  if(watcher->flags & WATCHER_STARTED)
  {
    ev_io_stop(evloop_thread_self()->ev_loop, __get_ev_io(watcher, target));
  }
}

///////////////////////////////////////////////////////////////////////////////
//
// libev callback
//
///////////////////////////////////////////////////////////////////////////////
static void
__ev_io_rx_callback(EV_P_ ev_io* w, int revents)
{
  watcher_t* watcher = container_of(w, watcher_t, rx_io);

  watcher->cb(watcher, watcher_event_rx);
}

static void
__ev_io_tx_callback(EV_P_ ev_io* w, int revents)
{
  watcher_t* watcher = container_of(w, watcher_t, tx_io);

  watcher->cb(watcher, watcher_event_tx);
}

static void
__ev_io_er_callback(EV_P_ ev_io* w, int revents)
{
  watcher_t* watcher = container_of(w, watcher_t, er_io);

  watcher->cb(watcher, watcher_event_error);
}

void
watcher_init_with_fd(watcher_t* watcher, int fd, watcher_callback cb)
{
  watcher->fd       = fd;
  watcher->flags    = 0;
  watcher->cb       = cb;

  ev_io_init(&watcher->rx_io, __ev_io_rx_callback, fd, EV_READ);
  ev_io_init(&watcher->tx_io, __ev_io_tx_callback, fd, EV_WRITE);
  ev_io_init(&watcher->er_io, __ev_io_er_callback, fd, EV_EXCEPT);
}

void
watcher_start(watcher_t* watcher)
{
  if(watcher->flags & WATCHER_STARTED)
  {
    return;
  }

  watcher->flags |= WATCHER_STARTED;

  if(watcher->flags & WATCHER_RX_WATCH)
  {
    ev_io_start(evloop_thread_self()->ev_loop, &watcher->rx_io);
  }

  if(watcher->flags & WATCHER_TX_WATCH)
  {
    ev_io_start(evloop_thread_self()->ev_loop, &watcher->tx_io);
  }

  if(watcher->flags & WATCHER_ER_WATCH)
  {
    ev_io_start(evloop_thread_self()->ev_loop, &watcher->er_io);
  }
}

void
watcher_stop(watcher_t* watcher)
{
  if(!(watcher->flags & WATCHER_STARTED))
  {
    return;
  }

  watcher->flags &= ~WATCHER_STARTED;

  if(watcher->flags & WATCHER_RX_WATCH)
  {
    ev_io_stop(evloop_thread_self()->ev_loop, &watcher->rx_io);
  }

  if(watcher->flags & WATCHER_TX_WATCH)
  {
    ev_io_stop(evloop_thread_self()->ev_loop, &watcher->tx_io);
  }

  if(watcher->flags & WATCHER_ER_WATCH)
  {
    ev_io_stop(evloop_thread_self()->ev_loop, &watcher->er_io);
  }
}

void
watcher_watch_rx(watcher_t* watcher)
{
  __watcher_start(watcher, WATCHER_RX_WATCH);
}

void
watcher_no_watch_rx(watcher_t* watcher)
{
  __watcher_stop(watcher, WATCHER_RX_WATCH);
}

void
watcher_watch_tx(watcher_t* watcher)
{
  __watcher_start(watcher, WATCHER_TX_WATCH);
}

void
watcher_no_watch_tx(watcher_t* watcher)
{
  __watcher_stop(watcher, WATCHER_TX_WATCH);
}

void
watcher_watch_err(watcher_t* watcher)
{
  __watcher_start(watcher, WATCHER_ER_WATCH);
}

void
watcher_no_watch__err(watcher_t* watcher)
{
  __watcher_stop(watcher, WATCHER_ER_WATCH);
}

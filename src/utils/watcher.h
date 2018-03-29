#ifndef __WATCHER_DEF_H__
#define __WATCHER_DEF_H__

#include <ev.h>
#include <stdint.h>
#include "common_def.h"

struct __watcher;
typedef struct __watcher watcher_t;

typedef enum
{
  watcher_event_rx,
  watcher_event_tx,
  watcher_event_error,
} watcher_event_t;

#define WATCHER_RX_WATCH          0x01
#define WATCHER_TX_WATCH          0x02
#define WATCHER_ER_WATCH          0x04
#define WATCHER_STARTED           0x08

typedef void (*watcher_callback)(watcher_t* watcher, watcher_event_t);

struct __watcher
{
  int                   fd;

  uint8_t               flags;

  watcher_callback      cb;

  ev_io                 rx_io;
  ev_io                 tx_io;
  ev_io                 er_io;
};

extern void watcher_init_with_fd(watcher_t* watcher, int fd, watcher_callback cb);
extern void watcher_start(watcher_t* watcher);
extern void watcher_stop(watcher_t* watcher);
extern void watcher_watch_rx(watcher_t* watcher);
extern void watcher_no_watch_rx(watcher_t* watcher);
extern void watcher_watch_tx(watcher_t* watcher);
extern void watcher_no_watch_tx(watcher_t* watcher);
extern void watcher_watch_err(watcher_t* watcher);
extern void watcher_no_watch_err(watcher_t* watcher);

#endif /* !__WATCHER_DEF_H__ */

#ifndef __TCP_CONNECTOR_DEF_H__
#define __TCP_CONNECTOR_DEF_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>

#include "evloop_timer.h"
#include "watcher.h"
#include "common_def.h"

typedef enum
{
  tcp_connector_success,
  tcp_connector_failure,
  tcp_connector_in_progress,
  tcp_connector_timeout,
} tcp_connector_status_t;

struct __tcp_connector;
typedef struct __tcp_connector tcp_connector_t;

typedef void (*tcp_connector_callback)(tcp_connector_t* conn, tcp_connector_status_t status);

struct __tcp_connector
{
  int                       sd;
  bool                      is_in_prog;
  tcp_connector_callback    cb;

  evloop_timer_t            tmr;
  watcher_t                 watcher;
};

extern void tcp_connector_deinit(tcp_connector_t* conn);
extern tcp_connector_status_t tcp_connector_try_ipv4(tcp_connector_t* conn, struct sockaddr_in* addr, double to);
extern tcp_connector_status_t tcp_connectyr_try_unix_domain(tcp_connector_t* conn, struct sockaddr_un* addr, double to);
extern int tcp_connect_get_sd(tcp_connector_t* conn);

#endif /* !__TCP_CONNECTOR_DEF_H__ */

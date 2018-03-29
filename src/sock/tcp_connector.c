#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "list.h"
#include "trace.h"
#include "tcp_connector.h"
#include "sock_util.h"

///////////////////////////////////////////////////////////////////////////////
//
// watcher callback
//
///////////////////////////////////////////////////////////////////////////////
static void
__tcp_connector_watcher_callback(watcher_t* watcher, watcher_event_t evt)
{
  tcp_connector_t*        conn = container_of(watcher, tcp_connector_t, watcher);
  tcp_connector_status_t  status = tcp_connector_failure;
  int                     err;
  socklen_t               len = sizeof(err);

  getsockopt(conn->sd, SOL_SOCKET, SO_ERROR, &err, &len);
  if(err == 0)
  {
    status = tcp_connector_success;
  }

  conn->is_in_prog = FALSE;

  watcher_stop(&conn->watcher);
  evloop_timer_stop(&conn->tmr);

  conn->cb(conn, status);
}

///////////////////////////////////////////////////////////////////////////////
//
// timeout callback
//
///////////////////////////////////////////////////////////////////////////////
static void
__tcp_connector_timeout(evloop_timer_t* t, void* arg)
{
  tcp_connector_t* conn = container_of(t, tcp_connector_t, tmr);

  conn->is_in_prog = FALSE;
  watcher_stop(&conn->watcher);

  conn->cb(conn, tcp_connector_timeout);
}

///////////////////////////////////////////////////////////////////////////////
//
// utilities
//
///////////////////////////////////////////////////////////////////////////////
static void
tcp_connector_init(tcp_connector_t* conn, int sd)
{
  conn->sd          = sd;
  conn->is_in_prog  = FALSE;

  watcher_init_with_fd(&conn->watcher, sd, __tcp_connector_watcher_callback);
  watcher_watch_tx(&conn->watcher);

  evloop_timer_init(&conn->tmr, __tcp_connector_timeout, NULL);
}

static tcp_connector_status_t
tcp_connector_connect(tcp_connector_t* conn, struct sockaddr* addr, socklen_t addrlen, double to)
{
  int ret;

  if(conn->is_in_prog)
  {
    return tcp_connector_failure;
  }

  ret = connect(conn->sd, addr, addrlen);
  if(ret == 0)
  {
    // immediately connected
    // this can happen in case of connection on the same host
    // using unix domain protocol
    return tcp_connector_success;
  }

  if(errno != EINPROGRESS)
  {
    // failure occurred for some reason
    return tcp_connector_failure;
  }

  conn->is_in_prog = TRUE;
  watcher_start(&conn->watcher);
  evloop_timer_start(&conn->tmr, to, 0);

  return tcp_connector_in_progress;
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
void
tcp_connector_deinit(tcp_connector_t* conn)
{
  if(conn->is_in_prog)
  {
    conn->is_in_prog = FALSE;
    watcher_stop(&conn->watcher);
    evloop_timer_stop(&conn->tmr);      // doesn't hurt to stop again
  }

  if(conn->sd != -1)
  {
    close(conn->sd);
    conn->sd = -1;
  }
}

tcp_connector_status_t
tcp_connector_try_ipv4(tcp_connector_t* conn, struct sockaddr_in* addr, double to)
{
  int         sd;

  conn->sd = -1;
  sd = socket(AF_INET, SOCK_STREAM, 0);

  if(sd < 0)
  {
    TRACE(SOCK_ERR, "socket() failed: %d\n", errno);
    return tcp_connector_failure;
  }

  sock_util_put_nonblock(sd);

  tcp_connector_init(conn, sd);

  return tcp_connector_connect(conn, (struct sockaddr*)addr, sizeof(struct sockaddr_in), to);
}

tcp_connector_status_t
tcp_connectyr_try_unix_domain(tcp_connector_t* conn, struct sockaddr_un* addr, double to)
{
  int         sd;

  conn->sd = -1;
  sd = socket(AF_UNIX, SOCK_STREAM, 0);

  if(sd < 0)
  {
    TRACE(SOCK_ERR, "socket() failed: %d\n", errno);
    return tcp_connector_failure;
  }

  sock_util_put_nonblock(sd);

  tcp_connector_init(conn, sd);

  return tcp_connector_connect(conn, (struct sockaddr*)addr, sizeof(struct sockaddr_un), to);
}

int
tcp_connect_get_sd(tcp_connector_t* conn)
{
  int sd = conn->sd;

  conn->sd = -1;

  return sd;
}

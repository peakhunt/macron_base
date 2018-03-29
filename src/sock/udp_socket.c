#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

#include "list.h"
#include "trace.h"
#include "udp_socket.h"
#include "sock_util.h"

static void
udp_watcher_callback(watcher_t* watcher, watcher_event_t evt)
{
  int                 ret;
  udp_socket_t*       sock = container_of(watcher, udp_socket_t, watcher);
  udp_socket_event_t  uevt;
  socklen_t           from_len;

  switch(evt)
  {
  case watcher_event_rx:
    from_len = sock->from_len;
    ret = recvfrom(sock->watcher.fd, sock->rx_buf, sock->rx_buf_size, 0,
        sock->from, &from_len);

    if(ret > 0)
    {
      sock->rx_data_len = ret;
      uevt = udp_socket_event_rx;
    }
    else
    {
      uevt = udp_socket_event_err;
    }
    break;

  case watcher_event_tx:
    uevt = udp_socket_event_tx_buf_avail;
    watcher_no_watch_tx(&sock->watcher);
    break;

  case watcher_event_error:
    uevt = udp_socket_event_err;
    break;
  }

  sock->cb(sock, uevt);
}

void
udp_socket_init(udp_socket_t* sock, int sd, uint8_t* rx_buf, int rx_buf_size, int from_len)
{
  sock_util_put_nonblock(sd);

  sock->rx_buf        = rx_buf;
  sock->rx_buf_size   = rx_buf_size;
  sock->rx_data_len   = 0;

  watcher_init_with_fd(&sock->watcher, sd, udp_watcher_callback);

  watcher_watch_rx(&sock->watcher);
  watcher_watch_tx(&sock->watcher);
  watcher_watch_err(&sock->watcher);

  sock->from    = (struct sockaddr*)&sock->from_ipv4;
  sock->from_len = from_len;
}

void
udp_socket_deinit(udp_socket_t* sock)
{
  udp_socket_stop(sock);
  close(sock->watcher.fd);
}

void
udp_socket_start(udp_socket_t* sock)
{
  watcher_start(&sock->watcher);
}

void
udp_socket_stop(udp_socket_t* sock)
{
  watcher_stop(&sock->watcher);
}

bool
udp_socket_send(udp_socket_t* sock, uint8_t* data, int len, struct sockaddr* to, socklen_t addr_len)
{
  ssize_t ret;

  ret = sendto(sock->watcher.fd, data, len, 0, to, addr_len);
  if(ret > 0)
  {
    TRACE(SOCK_ERR, "sendto() failed: %d\n", errno);
    return TRUE;
  }

  if(errno == EWOULDBLOCK || errno == EAGAIN)
  {
    watcher_watch_tx(&sock->watcher);
  }

  return FALSE;
}

bool
udp_socket_ipv4_init(udp_socket_t* sock, int port, uint8_t* rx_buf, int rx_buf_size)
{
  int                 sd;
  struct sockaddr_in  my_addr;

  sd = socket(AF_INET, SOCK_DGRAM, 0);
  if(sd < 0)
  {
    TRACE(SOCK_ERR, "socket() failed: %d\n", errno);
    return FALSE;
  }
  sock_util_put_nonblock(sd);

  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sin_family        = AF_INET;
  my_addr.sin_addr.s_addr   = INADDR_ANY;
  my_addr.sin_port          = htons(port);

  if(bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr)) != 0)
  {
    TRACE(SOCK_ERR, "bind() failed: %d\n", errno);
    close(sd);
    return FALSE;
  }

  udp_socket_init(sock, sd, rx_buf, rx_buf_size, sizeof(struct sockaddr_in));
  return TRUE;
}

bool
udp_socket_unix_domain_init(udp_socket_t* sock, const char* path, uint8_t* rx_buf, int rx_buf_size)
{
  int                 sd;
  struct sockaddr_un   my_addr;

  sd = socket(AF_UNIX, SOCK_DGRAM, 0);
  if(sd < 0)
  {
    TRACE(SOCK_ERR, "socket() failed: %d\n", errno);
    return FALSE;
  }
  sock_util_put_nonblock(sd);

  memset(&my_addr, 0, sizeof(my_addr));
  my_addr.sun_family      = AF_UNIX;
  snprintf(my_addr.sun_path, sizeof(my_addr.sun_path), "%s", path);

  if(bind(sd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_un)) != 0)
  {
    TRACE(SOCK_ERR, "bind() failed: %d\n", errno);
    close(sd);
    return FALSE;
  }

  udp_socket_init(sock, sd, rx_buf, rx_buf_size, sizeof(struct sockaddr_un));
  return TRUE;
}

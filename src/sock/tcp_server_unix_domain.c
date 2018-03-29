#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>

#include "list.h"
#include "trace.h"
#include "sock_util.h"
#include "tcp_server_unix_domain.h"

///////////////////////////////////////////////////////////////////////////////
//
// rx event handler on server socket
//
///////////////////////////////////////////////////////////////////////////////
static void
tcp_server_unix_domain_rx_event(tcp_server_t* self)
{
  struct sockaddr_un    from;
  socklen_t             from_len;
  int                   newsd;

  from_len = sizeof(struct sockaddr_un);
  memset(&from, 0, from_len);

  newsd = accept(self->sd, (struct sockaddr*)&from, &from_len);
  if(newsd < 0)
  {
    TRACE(SOCK_ERR, "accept() failed: %d\n", errno);
    return;
  }

  self->conn_cb(self, newsd, (struct sockaddr*)&from);
}

///////////////////////////////////////////////////////////////////////////////
//
// bound address info for debugging
//
///////////////////////////////////////////////////////////////////////////////
static void
tcp_server_unix_domain_get_bound_addr(tcp_server_t* self, char string[MAX_ADDRESS_STRING_LEN])
{
  snprintf(string, MAX_ADDRESS_STRING_LEN - 1, "unix_tcp:%s", self->unix_addr.sun_path);
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
int
tcp_server_unix_domain_init(tcp_server_t* server, const char* path, int backlog)
{
  int         sd;
  const int   on = 1;

  server->server_type   = tcp_server_type_unix_domain;

  server->sd = -1;

  sd = socket(AF_INET, SOCK_STREAM, 0);

  if(sd < 0)
  {
    TRACE(SOCK_ERR, "socket() failed: %d\n", errno);
    return -1;
  }

  sock_util_put_nonblock(sd);

  memset(&server->unix_addr, 0, sizeof(struct sockaddr_un));

  server->unix_addr.sun_family      = AF_UNIX;
  snprintf(server->unix_addr.sun_path, sizeof(server->unix_addr.sun_path), "%s", path);
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  if(bind(sd, (struct sockaddr*)&server->unix_addr, sizeof(struct sockaddr_un)) != 0)
  {
    TRACE(SOCK_ERR, "bind() failed: %d\n", errno);
    close(sd);
    return -1;
  }

  if(listen(sd, backlog) != 0)
  {
    TRACE(SOCK_ERR, "listen() failed: %d\n", errno);
    close(sd);
    return -1;
  }

  tcp_server_init(server, sd, tcp_server_unix_domain_rx_event);

  server->get_bound_addr    = tcp_server_unix_domain_get_bound_addr;

  return -1;
}

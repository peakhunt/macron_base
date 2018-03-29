#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "tcp_server.h"
#include "list.h"

///////////////////////////////////////////////////////////////////////////////
//
// watcher callback
//
///////////////////////////////////////////////////////////////////////////////
static void
__tcp_server_watcher_callback(watcher_t* watcher, watcher_event_t evt)
{
  tcp_server_t*   server = container_of(watcher, tcp_server_t, watcher);

  server->rx_event(server);
}

///////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
///////////////////////////////////////////////////////////////////////////////
void
tcp_server_init(tcp_server_t* server, int sd, tcp_server_rx_event cb)
{
  server->sd            = sd;
  server->is_listening  = FALSE;
  server->rx_event      = cb;

  watcher_init_with_fd(&server->watcher, sd, __tcp_server_watcher_callback);
  watcher_watch_rx(&server->watcher);
}

void
tcp_server_deinit(tcp_server_t* server)
{
  tcp_server_stop(server);

  if(server->sd != -1)
  {
    close(server->sd);
    server->sd = -1;
  }
}

void
tcp_server_start(tcp_server_t* server)
{
  if(server->is_listening)
  {
    return;
  }

  server->is_listening = TRUE;
  watcher_start(&server->watcher);
}

void
tcp_server_stop(tcp_server_t* server)
{
  if(!server->is_listening)
  {
    return;
  }

  server->is_listening = FALSE;
  watcher_stop(&server->watcher);
}

void
tcp_server_get_port_name(tcp_server_t* server, char name[MAX_ADDRESS_STRING_LEN])
{
  server->get_bound_addr(server, name);
}

#ifndef __UDP_SOCKET_DEF_H__
#define __UDP_SOCKET_DEF_H__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include "common_def.h"
#include "watcher.h"

struct __udp_socket;
typedef struct __udp_socket udp_socket_t;

typedef enum
{
  udp_socket_event_rx,
  udp_socket_event_err,
  udp_socket_event_tx_buf_avail,
} udp_socket_event_t;

typedef void (*udp_callback)(udp_socket_t* sock, udp_socket_event_t evt);

struct __udp_socket
{
  watcher_t           watcher;
  uint8_t*            rx_buf;
  int                 rx_buf_size;
  int                 rx_data_len;
  union
  {
    struct sockaddr_in    from_ipv4;
    struct sockaddr_un    from_unix;
  };
  struct sockaddr*        from;
  socklen_t               from_len;
  udp_callback        cb;
};

extern void udp_socket_init(udp_socket_t* sock, int sd, uint8_t* rx_buf, int rx_buf_size, int from_len);
extern void udp_socket_deinit(udp_socket_t* sock);
extern void udp_socket_start(udp_socket_t* sock);
extern void udp_socket_stop(udp_socket_t* sock);
extern bool udp_socket_send(udp_socket_t* sock, uint8_t* data, int len, struct sockaddr* to, socklen_t addr_len); 
extern bool udp_socket_ipv4_init(udp_socket_t* sock, int port, uint8_t* rx_buf, int rx_buf_size);
extern bool udp_socket_unix_domain_init(udp_socket_t* sock, const char* path, uint8_t* rx_buf, int rx_buf_size);

#endif /* !__UDP_SOCKET_DEF_H__ */

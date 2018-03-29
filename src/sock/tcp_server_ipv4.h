#ifndef __TCP_SERVER_IPV4_DEF_H__
#define __TCP_SERVER_IPV4_DEF_H__

#include "tcp_server.h"

extern int tcp_server_ipv4_init(tcp_server_t* server, int port, int backlog);
extern int tcp_server_ipv4_init_with_addr(tcp_server_t* server, struct sockaddr_in* addr, int backlog);

#endif /* !__TCP_SERVER_IPV4_DEF_H__ */

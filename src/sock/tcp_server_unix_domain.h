#ifndef __TCP_SERVER_UNIX_DOMAIN_DEF_H__
#define __TCP_SERVER_UNIX_DOMAIN_DEF_H__

#include "tcp_server.h"

extern int tcp_server_unix_domain_init(tcp_server_t* server, const char* path, int backlog);

#endif /* !__TCP_SERVER_UNIX_DOMAIN_DEF_H__ */

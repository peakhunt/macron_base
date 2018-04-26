#ifndef __TCP_AUTO_CONNECTOR_DEF_H__
#define __TCP_AUTO_CONNECTOR_DEF_H__

#include <stdint.h>
#include "tcp_connector.h"

struct __tcp_auto_connector;
typedef struct __tcp_auto_connector tcp_auto_connector_t;

typedef void (*tcp_auto_connector_callback)(tcp_auto_connector_t* conn, int sd);

typedef enum
{
  tcp_auto_connector_state_not_started,
  tcp_auto_connector_state_connecting,
  tcp_auto_connector_state_reconnect_wait,
} tcp_auto_connector_state_t;;

struct __tcp_auto_connector
{
  tcp_connector_t               tcp_connector;
  struct sockaddr_in            server_addr;

  evloop_timer_t                reconn_wait_tmr;

  tcp_auto_connector_callback   cb;

  double                        conn_tmr_value;
  double                        reconn_wait_tmr_value;

  tcp_auto_connector_state_t    state;

  uint32_t      n_conn_attempt;
  uint32_t      n_conn_success;
  uint32_t      n_conn_fail;
};

extern void tcp_auto_connector_init(tcp_auto_connector_t* conn, struct sockaddr_in* server_addr, double conn_tmr, double reconn_wait_tmr, bool reset_stat);
extern void tcp_auto_connector_start(tcp_auto_connector_t* conn);
extern void tcp_auto_connector_stop(tcp_auto_connector_t* conn);

#endif /* !__TCP_AUTO_CONNECTOR_DEF_H__ */

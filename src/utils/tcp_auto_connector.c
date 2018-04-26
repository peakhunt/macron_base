#include "trace.h"
#include "tcp_auto_connector.h"

static void tcp_auto_connector_connect(tcp_auto_connector_t* conn);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  private utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
tcp_auto_connector_handle_tcp_connect_event(tcp_auto_connector_t* conn, tcp_connector_status_t status)
{
  switch(status)
  {
  case tcp_connector_success:
    conn->n_conn_success++;
    conn->state = tcp_auto_connector_state_not_started;
    conn->cb(conn, tcp_connect_get_sd(&conn->tcp_connector));
    break;

  case tcp_connector_failure:
    conn->n_conn_fail++;
    tcp_connector_deinit(&conn->tcp_connector);

    conn->state = tcp_auto_connector_state_reconnect_wait;
    evloop_timer_start(&conn->reconn_wait_tmr, conn->reconn_wait_tmr_value, 0);
    conn->cb(conn, -1);
    break;

  case tcp_connector_in_progress:
    conn->state = tcp_auto_connector_state_connecting;
    break;

  case tcp_connector_timeout:
    conn->n_conn_fail++;
    tcp_connector_deinit(&conn->tcp_connector);
    conn->cb(conn, -1);
    tcp_auto_connector_connect(conn);
    break;
  }
}

static void
tcp_auto_connector_connect(tcp_auto_connector_t* conn)
{
  tcp_connector_status_t    ret;

  conn->n_conn_attempt++;

  ret = tcp_connector_try_ipv4(&conn->tcp_connector, &conn->server_addr, conn->conn_tmr_value);
  tcp_auto_connector_handle_tcp_connect_event(conn, ret);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// callbacks
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void
tcp_auto_connector_reconn_wait_timeout(evloop_timer_t* te, void* unused)
{
  tcp_auto_connector_t* conn = container_of(te, tcp_auto_connector_t, reconn_wait_tmr);

  tcp_auto_connector_connect(conn);
}

static void
tcp_auto_connector_tcp_connector_callback(tcp_connector_t* tconn,
    tcp_connector_status_t status)
{
  tcp_auto_connector_t* conn = container_of(tconn, tcp_auto_connector_t, tcp_connector);

  tcp_auto_connector_handle_tcp_connect_event(conn, status);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  public interfaces
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
tcp_auto_connector_init(tcp_auto_connector_t* conn, struct sockaddr_in* server_addr, double conn_tmr, double reconn_wait_tmr, bool reset_stat)
{
  memcpy(&conn->server_addr, server_addr, sizeof(struct sockaddr_in));

  conn->conn_tmr_value        = conn_tmr;
  conn->reconn_wait_tmr_value = reconn_wait_tmr;

  conn->tcp_connector.cb    = tcp_auto_connector_tcp_connector_callback;

  evloop_timer_init(&conn->reconn_wait_tmr, tcp_auto_connector_reconn_wait_timeout, NULL);

  conn->state = tcp_auto_connector_state_not_started;

  if(reset_stat)
  {
    conn->n_conn_attempt  = 0;
    conn->n_conn_success  = 0;
    conn->n_conn_fail     = 0;
  }
}

void
tcp_auto_connector_start(tcp_auto_connector_t* conn)
{
  tcp_auto_connector_connect(conn);
}

void
tcp_auto_connector_stop(tcp_auto_connector_t* conn)
{
  switch(conn->state)
  {
  case tcp_auto_connector_state_not_started:
  case tcp_auto_connector_state_connecting:
    tcp_connector_deinit(&conn->tcp_connector);
    break;

  case tcp_auto_connector_state_reconnect_wait:
    tcp_connector_deinit(&conn->tcp_connector);
    evloop_timer_stop(&conn->reconn_wait_tmr);
    break;
  }

  conn->state = tcp_auto_connector_state_not_started;
}

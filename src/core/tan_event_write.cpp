/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_events.h"


static void tan_ws_handshake_continue(tan_connection_t *conn);
static void tan_send_packet_continue(tan_connection_t *conn);


void
tan_event_write(tan_connection_t *conn)
{
    if (conn->status.flags & TAN_CONN_STATUS_WS_HANDSHAKING)
        tan_ws_handshake_continue(conn);

    if (conn->status.flags & TAN_CONN_STATUS_WRITE_PENDING)
        tan_send_packet_continue(conn);
}


static void
tan_ws_handshake_continue(tan_connection_t *conn)
{
    if (tan_ssl_write(conn->info.ssl,
                      conn->event.ws_response_header.c_str(),
                      conn->event.ws_response_header.length())
        == TAN_SSL_WRITE_OK)
    {
        conn->status.flags &= ~TAN_CONN_STATUS_WS_HANDSHAKING;
    }
}


static void
tan_send_packet_continue(tan_connection_t *conn)
{
    if (tan_ssl_write(conn->info.ssl,
                      conn->event.packet.c_str(),
                      conn->event.packet.length())
        == TAN_SSL_WRITE_OK)
    {
        conn->status.flags &= ~TAN_CONN_STATUS_WRITE_PENDING;

        tan_handled_requests_atomic_inc();
        tan_write_access_log(conn);
    }
}

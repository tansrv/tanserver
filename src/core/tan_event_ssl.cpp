/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_events.h"


void
tan_event_ssl_handshake(tan_connection_t *conn)
{
    if (conn->status.flags & TAN_CONN_STATUS_SSL_HANDSHAKING) {

        if (tan_ssl_accept(conn->info.ssl)
            != TAN_SSL_ACCEPT_OK)
        {
            return;
        } else {
            /* Handshake succeeded.  */
            conn->status.flags &= ~TAN_CONN_STATUS_SSL_HANDSHAKING;
        }
    }

    tan_event_select_protocol(conn);
}

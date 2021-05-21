/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_events.h"


void
tan_event_ssl_handshake(tan_connection_t *conn)
{
    if (tan_ssl_accept(conn->info.ssl)
        != TAN_SSL_ACCEPT_OK)
    {
        return;
    }

    tan_event_get_protocol(conn);
}

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_events.h"
#include "tan_connection.h"


void
tan_event_write(tan_connection_t *conn)
{
    if (!(conn->status.write_pending & 1))
        return;

    if (tan_ssl_write(conn->info.ssl, conn->event.packet.c_str(),
                      conn->event.packet.length())
        == TAN_SSL_WRITE_OK)
    {
        tan_handled_requests_atomic_inc();
        tan_write_access_log(conn);
    }
}

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_events.h"
#include "tan_connection.h"


void
tan_event_close(tan_connection_t *conn)
{
    if (conn->status.flags & TAN_CONN_STATUS_CLOSING)
        tan_free_client_connection(conn);
}

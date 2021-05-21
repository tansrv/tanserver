/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_events.h"


static tan_ssl_t tan_get_protocol(tan_connection_t *conn, char *protocol);
static tan_int_t tan_select_protocol(tan_connection_t *conn);


void
tan_event_get_protocol(tan_connection_t *conn)
{
    char       protocol[2];
    tan_ssl_t  ret;

    tan_memzero(protocol, 2);

    ret = tan_get_protocol(conn, protocol);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_get_protocol;
        return;

    default:

        goto out_disconnect;
    }

    conn->info.protocol = protocol[0];

    if (tan_select_protocol(conn) != TAN_OK)
        goto out_disconnect;

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static tan_ssl_t
tan_get_protocol(tan_connection_t *conn, char *protocol)
{
    int  bytes_read;

    bytes_read = 0;

    return tan_ssl_read(&bytes_read, conn->info.ssl,
                        protocol, 1);
}


static tan_int_t
tan_select_protocol(tan_connection_t *conn)
{
    const u_char  *p;

    switch (conn->info.protocol) {

    case '0':

        tan_event_protocol_v0(conn);
        return TAN_OK;

    default:

        p = tan_get_hostaddr(&conn->info.addr);

        tan_log_info("invalid protocol: %c, client: %d.%d.%d.%d",
                     conn->info.protocol,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }
}

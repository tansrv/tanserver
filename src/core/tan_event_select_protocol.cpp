/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_events.h"
#include "tan_connection.h"
#include "tan_openssl_ssl.h"


static void tan_select_protocol(tan_connection_t *conn);


void
tan_event_select_protocol(tan_connection_t *conn)
{
    tan_ssl_t      ret;
    const u_char  *p;

    ret = tan_ssl_readline(conn->info.ssl, conn->event.header);

    switch (ret) {

    case TAN_SSL_READLINE_CONTINUE:

        conn->event.read = tan_event_select_protocol;
        return;

    case TAN_SSL_READLINE_DONE:

        break;

    case TAN_SSL_READLINE_TOO_LARGE:

        p = tan_get_hostaddr(&conn->info.addr);

        tan_log_info(TAN_REQUEST_HEADER_TOO_LARGE,
                     p[0], p[1], p[2], p[3]);

    case TAN_SSL_READLINE_FAILED:

        goto out_disconnect;
    }

    if (tan_unlikely(conn->status.flags & TAN_CONN_STATUS_CLOSING))
        goto out_disconnect;

    tan_select_protocol(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static void
tan_select_protocol(tan_connection_t *conn)
{
    const char  *header;

    header = conn->event.header.c_str();

    if (header[0] == 'G' && header[1] == 'E' && header[2] == 'T') {

        conn->info.protocol = 1;
        tan_event_websocket(conn);

    } else {

        conn->info.protocol = 0;
        tan_event_custom_protocol(conn);
    }
}

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_custom_protocol.h"
#include "tan_events.h"


static void tan_event_recv_json(tan_connection_t *conn);
static void tan_make_packet(std::string &json_string);


void
tan_event_custom_protocol(tan_connection_t *conn)
{
    if (tan_parse_custom_protocol_header(conn, conn->event.header.c_str())
        != TAN_OK)
    {
        goto out_disconnect;
    }

    conn->event.content.reset(new
        char[conn->event.content_length + 1]());

    tan_event_recv_json(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static void
tan_event_recv_json(tan_connection_t *conn)
{
    tan_ssl_t  ret;

    ret = tan_ssl_read(&conn->event.content_read, conn->info.ssl,
                       conn->event.content.get(),
                       conn->event.content_length);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_recv_json;
        return;

    default:

        goto out_disconnect;
    }

    /*
     * After receiving the JSON data, we need to
     * parse it and call the user API. The returned string
     * will be stored in conn->event.packet.
     */
    if (tan_parse_custom_protocol_body_and_call_api(conn, conn->event.content.get())
        != TAN_OK)
    {
        goto out_disconnect;
    }

    tan_make_packet(conn->event.packet);

    tan_connection_send_packet(conn);

    conn->event.read    = tan_event_close;
    conn->status.flags |= TAN_CONN_STATUS_CLOSING;

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static void
tan_make_packet(std::string &json_string)
{
    json_string.append("\r\n");
}

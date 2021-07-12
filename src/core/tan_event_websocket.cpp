/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_events.h"
#include "tan_http.h"
#include "tan_jsoncpp.h"
#include "tan_user_api.h"
#include "tan_connection.h"
#include "tan_openssl_ssl.h"
#include "tan_websocket.h"


/* length <= 125  */
#define TAN_WS_GET_PAYLOAD_LEN_7(buf)                 \
    (buf[0] & 0x7f)

/* length > 125 && length <= 65535  */
#define TAN_WS_GET_PAYLOAD_LEN_7_16(buf)              \
    ((buf[0] & 0xff << 8) | (buf[1] & 0xff))

/* length > 65535  */
#define TAN_WS_GET_PAYLOAD_LEN_7_64(buf)              \
    ((buf[4] & 0xff << 24) | (buf[5] & 0xff << 16) |  \
        (buf[6] & 0xff << 8) | (buf[7] & 0xff))


typedef enum {
    TAN_WS_READ_HEADER_FAILED = 0,
    TAN_WS_READ_HEADER_CONTINUE,
    TAN_WS_READ_HEADER_DONE,
} tan_ws_read_header_status_e;


static int tan_ws_recv_header(tan_connection_t *conn);
static tan_int_t tan_ws_handshake(tan_connection_t *conn);
static void tan_ws_make_response_header(char *buf,
                                        const char *sec_ws_accept);


void
tan_event_websocket(tan_connection_t *conn)
{
    int  ret;

    ret = tan_ws_recv_header(conn);

    switch (ret) {

    case TAN_WS_READ_HEADER_CONTINUE:

        conn->event.read = tan_event_websocket;
        return;

    case TAN_WS_READ_HEADER_DONE:

        break;

    case TAN_WS_READ_HEADER_FAILED:

        goto out_disconnect;
    }

    if (tan_ws_handshake(conn) != TAN_OK)
        goto out_disconnect;

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static int
tan_ws_recv_header(tan_connection_t *conn)
{
    tan_ssl_t  ret;

    for (;;) {

        ret = tan_ssl_readline(conn->info.ssl, conn->event.header);

        switch (ret) {

        case TAN_SSL_READLINE_CONTINUE:

            return TAN_WS_READ_HEADER_CONTINUE;

        case TAN_SSL_READLINE_DONE:

            break;

        case TAN_SSL_READLINE_FAILED:

            return TAN_WS_READ_HEADER_FAILED;
        }

        if (strstr(conn->event.header.c_str(), "\r\n\r\n") == NULL)
            continue;
        else
            return TAN_WS_READ_HEADER_DONE;
    }
}


static tan_int_t
tan_ws_handshake(tan_connection_t *conn)
{
    char        *sec_ws_accept, buf[TAN_MAX_STR_SIZE];
    std::string  key;

    try {
        key = tan_http_header_get_value(conn->event.header.c_str(),
                                        "Sec-WebSocket-Key");
    } catch (...) {
        return TAN_ERROR;
    }

    sec_ws_accept = tan_generate_sec_websocket_accept(key.c_str());

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    tan_ws_make_response_header(buf, sec_ws_accept);
    free(sec_ws_accept);

    conn->event.ws_response_header = buf;

    if (tan_ssl_write(conn->info.ssl,
                      conn->event.ws_response_header.c_str(),
                      conn->event.ws_response_header.length())
        == TAN_SSL_CONTINUE)
    {
        conn->status.flags |= TAN_CONN_STATUS_WS_HANDSHAKING;
    }

    return TAN_OK;
}


static void
tan_ws_make_response_header(char *buf,
                            const char *sec_ws_accept)
{
    snprintf(buf, TAN_MAX_STR_SIZE,
             TAN_WEBSOCKET_RESPONSE_HEADER, sec_ws_accept);
}

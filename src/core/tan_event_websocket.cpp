/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_events.h"
#include "tan_http.h"
#include "tan_connection.h"
#include "tan_openssl_ssl.h"
#include "tan_websocket.h"
#include "tan_custom_protocol.h"


/* length <= 125  */
#define TAN_WS_GET_PAYLOAD_LEN_7(buf)                 \
    (buf & 0x7f)

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


/* Websocket handshake.  */
static int tan_ws_recv_header(tan_connection_t *conn);
static tan_int_t tan_ws_handshake(tan_connection_t *conn);
static void tan_ws_make_response_header(char *buf,
                                        const char *sec_ws_accept);

/*
 * Receive a WebSocket frame.
 *
 * Frame format:
 *    0                   1                   2                   3
 *    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *   +-+-+-+-+-------+-+-------------+-------------------------------+
 *   |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
 *   |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
 *   |N|V|V|V|       |S|             |   (if payload len==126/127)   |
 *   | |1|2|3|       |K|             |                               |
 *   +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
 *   |     Extended payload length continued, if payload len == 127  |
 *   + - - - - - - - - - - - - - - - +-------------------------------+
 *   |                               |Masking-key, if MASK set to 1  |
 *   +-------------------------------+-------------------------------+
 *   | Masking-key (continued)       |          Payload Data         |
 *   +-------------------------------- - - - - - - - - - - - - - - - +
 *   :                     Payload Data continued ...                :
 *   + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 *   |                     Payload Data continued ...                |
 *   +---------------------------------------------------------------+
 */
static void tan_event_ws_recv_2_bytes(tan_connection_t *conn);
static void tan_event_ws_payload_len_larger_125_bytes(tan_connection_t *conn);
static void tan_event_ws_payload_len_larger_65535_bytes(tan_connection_t *conn);
static void tan_event_ws_recv_mask(tan_connection_t *conn);
static void tan_event_ws_recv_raw_data(tan_connection_t *conn);

static void tan_ws_decode_raw_data(tan_connection_t *conn,
                                   std::string &content);
static tan_int_t tan_get_custom_protocol_header(tan_connection_t *conn,
                                                char *header,
                                                const std::string &content);


void
tan_event_websocket(tan_connection_t *conn)
{
    int  ret;

    ret = tan_ws_recv_header(conn);

    switch (ret) {

    case TAN_WS_READ_HEADER_DONE:

        break;

    case TAN_WS_READ_HEADER_CONTINUE:

        conn->event.read = tan_event_websocket;
        return;

    case TAN_WS_READ_HEADER_FAILED:

        goto out_disconnect;
    }

    if (tan_ws_handshake(conn) != TAN_OK)
        goto out_disconnect;

    tan_event_ws_recv_2_bytes(conn);

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
    char          *sec_ws_accept, buf[TAN_MAX_STR_SIZE];
    std::string    key;
    const u_char  *p;

    try {
        key = tan_http_header_get_value(conn->event.header.c_str(),
                                        "Sec-WebSocket-Key");
    } catch (...) {

        p = tan_get_hostaddr(&conn->info.addr);

        tan_log_info(TAN_WEBSOCKET_ERROR_SEC_WEBSOCKET_KEY_NOT_FOUND,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    sec_ws_accept = tan_generate_sec_websocket_accept(key.c_str());

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    tan_ws_make_response_header(buf, sec_ws_accept);
    free(sec_ws_accept);

    conn->event.ws_response_header = buf;

    /* Send response header.  */
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


static void
tan_event_ws_recv_2_bytes(tan_connection_t *conn)
{
    int            byte_read;
    tan_ssl_t      ret;
    unsigned int   len;
    unsigned char  buf[3];

    buf[2] = '\0';

    byte_read = 0;

    ret = tan_ssl_read(&byte_read, conn->info.ssl, buf, 2);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_ws_recv_2_bytes;
        return;

    default:

        goto out_disconnect;
    }

    /* opcode should be 0x1 (text).  */
    if (tan_unlikely(buf[0] != 0x81))
        goto out_disconnect;

    len = TAN_WS_GET_PAYLOAD_LEN_7(buf[1]);

    if (len < 126) {
        conn->event.content_length = len;
    } else if (len == 126) {

        tan_event_ws_payload_len_larger_125_bytes(conn);
        return;
    } else {

        tan_event_ws_payload_len_larger_65535_bytes(conn);
        return;
    }

    tan_event_ws_recv_mask(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static void
tan_event_ws_payload_len_larger_125_bytes(tan_connection_t *conn)
{
    int        byte_read;
    char       buf[3];
    tan_ssl_t  ret;

    buf[2] = '\0';

    byte_read = 0;

    ret = tan_ssl_read(&byte_read, conn->info.ssl, buf, 2);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_ws_payload_len_larger_125_bytes;
        return;

    default:

        goto out_disconnect;
    }

    conn->event.content_length = TAN_WS_GET_PAYLOAD_LEN_7_16(buf);

    tan_event_ws_recv_mask(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static void
tan_event_ws_payload_len_larger_65535_bytes(tan_connection_t *conn)
{
    int        byte_read;
    char       buf[9];
    tan_ssl_t  ret;

    buf[8] = '\0';

    byte_read = 0;

    ret = tan_ssl_read(&byte_read, conn->info.ssl, buf, 8);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_ws_payload_len_larger_65535_bytes;
        return;

    default:

        goto out_disconnect;
    }

    conn->event.content_length = TAN_WS_GET_PAYLOAD_LEN_7_64(buf);

    tan_event_ws_recv_mask(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static void
tan_event_ws_recv_mask(tan_connection_t *conn)
{
    int            byte_read;
    tan_ssl_t      ret;
    const u_char  *p;

    conn->event.mask[4] = '\0';

    byte_read = 0;

    ret = tan_ssl_read(&byte_read, conn->info.ssl,
                       conn->event.mask, 4);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_ws_recv_mask;
        return;

    default:

        goto out_disconnect;
    }

    /* Check payload length.  */
    if (conn->event.content_length >
        tan_get_server_cfg()->client_max_body_size)
    {
        p = tan_get_hostaddr(&conn->info.addr);

        tan_log_info(TAN_WEBSOCKET_ERROR_PAYLOAD_LENGTH_TOO_LARGE,
                     conn->event.content_length,
                     tan_get_server_cfg()->client_max_body_size,
                     p[0], p[1], p[2], p[3]);

        goto out_disconnect;
    }

    conn->event.content.reset(new
        char[conn->event.content_length + 1]());

    tan_event_ws_recv_raw_data(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static void
tan_event_ws_recv_raw_data(tan_connection_t *conn)
{
    tan_ssl_t    ret;
    char         header[TAN_MAX_STR_SIZE];
    std::string  content;

    ret = tan_ssl_read(&conn->event.content_read,
                       conn->info.ssl,
                       conn->event.content.get(),
                       conn->event.content_length);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_ws_recv_raw_data;
        return;

    default:

        goto out_disconnect;
    }

    /* We got something like: {"user_api":"api","json_length":2}\r\n{}  */
    tan_ws_decode_raw_data(conn, content);

    tan_memzero(header, TAN_MAX_STR_SIZE);

    if (tan_get_custom_protocol_header(conn, header, content)
        != TAN_OK)
    {
        goto out_disconnect;
    }

    if (tan_parse_custom_protocol_header(conn, header)
        != TAN_OK)
    {
        goto out_disconnect;
    }

    if (tan_parse_custom_protocol_body_and_call_api(conn,
        content.c_str() + strlen(header)) != TAN_OK)
    {
        goto out_disconnect;
    }

    conn->event.packet = tan_ws_make_packet(conn->event.packet.c_str());

    tan_connection_send_packet(conn);

    conn->event.read    = tan_event_close;
    conn->status.flags |= TAN_CONN_STATUS_CLOSING;

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static void
tan_ws_decode_raw_data(tan_connection_t *conn,
                       std::string &content)
{
    int   k;
    char  buf[2];

    buf[1] = '\0';

    for (k = 0; k < conn->event.content_length; ++k) {

        buf[0] = conn->event.content[k] ^ conn->event.mask[k % 4];
        content.append(buf);
    }
}


static tan_int_t
tan_get_custom_protocol_header(tan_connection_t *conn,
                               char *header,
                               const std::string &content)
{
    int            index;
    const u_char  *p;

    index = content.find("\r\n");
    if (index >= TAN_MAX_STR_SIZE) {

        p = tan_get_hostaddr(&conn->info.addr);

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_REQUEST_HEADER_TOO_LARGE,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    memcpy(header, content.c_str(), index);
    return TAN_OK;
}

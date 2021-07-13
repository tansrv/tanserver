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


static int tan_ws_recv_header(tan_connection_t *conn);
static tan_int_t tan_ws_handshake(tan_connection_t *conn);
static void tan_ws_make_response_header(char *buf,
                                        const char *sec_ws_accept);
static void tan_event_ws_recv_2_bytes(tan_connection_t *conn);
static void tan_event_ws_payload_len_larger_125_bytes(tan_connection_t *conn);
static void tan_event_ws_payload_len_larger_65535_bytes(tan_connection_t *conn);
static void tan_event_ws_recv_mask(tan_connection_t *conn);
static void tan_event_ws_recv_raw_data(tan_connection_t *conn);
static void tan_ws_decode_raw_data(tan_connection_t *conn,
                                   std::string &content);
static tan_int_t tan_get_custom_protocol_header(char *buf,
                                                const std::string &content,
                                                int *header_index);
static tan_int_t tan_custom_protocol_header_parse(tan_connection_t *conn,
                                                  const std::string &content,
                                                  int *header_index);
static tan_int_t tan_custom_protocol_body_parse(tan_connection_t *conn,
                                                const char *body);
static void tan_send_packet(tan_connection_t *conn);


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
    char        *sec_ws_accept, buf[TAN_MAX_STR_SIZE];
    std::string  key;

    try {
        key = tan_http_header_get_value(conn->event.header.c_str(),
                                        "Sec-WebSocket-Key");
    } catch (...) {

        tan_log_info(TAN_WEBSOCKET_ERROR_SEC_WEBSOCKET_KEY_NOT_FOUND,
                     p[0], p[1], p[2], p[3]);

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


static void
tan_event_ws_recv_2_bytes(tan_connection_t *conn)
{
    int            byte_read;
    tan_ssl_t      ret;
    unsigned int   len;
    unsigned char  buf[3];

    buf[2]    = '\0';
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

    if (buf[0] != 0x81)
        goto out_disconnect;

    len = TAN_WS_GET_PAYLOAD_LEN_7(buf[1]);

    if (len < 126) {
        conn->event.content_length = len;
    } else if (len < 65536) {

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
    int           byte_read;
    char          buf[3];
    tan_ssl_t     ret;
    unsigned int  len;

    buf[2]    = '\0';
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
    int           byte_read;
    char          buf[9];
    tan_ssl_t     ret;
    unsigned int  len;

    buf[8]    = '\0';
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
    int        byte_read;
    tan_ssl_t  ret;

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

    if (conn->event.content_length >
        tan_get_server_cfg()->client_max_body_size)
    {
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
    int          header_index;
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

    if (tan_custom_protocol_header_parse(conn, content, &header_index)
        != TAN_OK)
    {
        goto out_disconnect;
    }

    if (tan_custom_protocol_body_parse(conn, content.c_str() + header_index)
        != TAN_OK)
    {
        goto out_disconnect;
    }

    tan_send_packet(conn);

    conn->event.read    = tan_event_select_protocol;
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
tan_custom_protocol_header_parse(tan_connection_t *conn,
                                 const std::string &content,
                                 int *header_index)
{
    char           header[TAN_MAX_STR_SIZE];
    tan_int_t      ret;
    Json::Value    value;
    const u_char  *p;

    tan_memzero(header, TAN_MAX_STR_SIZE);

    ret = tan_get_custom_protocol_header(header, content, header_index);
    if (ret != TAN_OK)
        return ret;

    p = tan_get_hostaddr(&conn->info.addr);

    try {
        value = json_decode(header);
    } catch (...) {

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_INVALID_REQUEST_HEADER,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    conn->event.user_api = value["user_api"].asString();
    if (conn->event.user_api.empty()) {

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_USER_API_NOT_FOUND,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    return TAN_OK;
}


static tan_int_t
tan_get_custom_protocol_header(char *buf,
                               const std::string &content,
                               int *header_index)
{
    *header_index = content.find("\r\n");
    if (*header_index >= TAN_MAX_STR_SIZE) {

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_REQUEST_HEADER_TOO_LARGE,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    memcpy(buf, content.c_str(), *header_index);
    return TAN_OK;
}


static tan_int_t
tan_custom_protocol_body_parse(tan_connection_t *conn,
                               const char *body)
{
    tan_int_t       ret;
    const u_char   *p;
    const char     *func;
    Json::Value     value;
    tan_user_api_t  api;

    p = tan_get_hostaddr(&conn->info.addr);

    func = conn->event.user_api.c_str();

    api = tan_get_user_api_handler(func);
    if (api == NULL) {

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_FUNCTION_NOT_FOUND,
                     func,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    try {
        value = json_decode(body);
    } catch (...) {

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_INVALID_JSON_STRING,
                     func,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    tan_set_current_connection(conn);

    try {
        conn->event.packet = tan_ws_make_packet(api(value).c_str());
    } catch (...) {
        return TAN_ERROR;
    }

    return TAN_OK;
}


static void
tan_send_packet(tan_connection_t *conn)
{
    tan_ssl_t  ret;

    ret = tan_ssl_write(conn->info.ssl, conn->event.packet.c_str(),
                        conn->event.packet.length());

    switch (ret) {

    case TAN_SSL_WRITE_OK:

        tan_handled_requests_atomic_inc();
        tan_write_access_log(conn);

        break;

    case TAN_SSL_CONTINUE:

        conn->status.flags |= TAN_CONN_STATUS_WRITE_PENDING;
    }
}

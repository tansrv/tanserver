/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_events.h"
#include "tan_jsoncpp.h"
#include "tan_user_api.h"


#define TAN_MAX_PACKET_HEADER_SIZE  1024


typedef enum {
    TAN_RECV_HEADER_FAILED = 0,
    TAN_RECV_HEADER_CONTINUE,
    TAN_RECV_HEADER_DONE,
} tan_header_status_e;


typedef enum {
    TAN_ERR_INVALID_PACKET_HEADER = 0,
    TAN_ERR_INVALID_USER_API,
    TAN_ERR_INVALID_JSON_LENGTH,
    TAN_ERR_FUNCTION_NOT_FOUND,
    TAN_ERR_INVALID_JSON_STRING,
} tan_error_num_e;


static int tan_recv_header(tan_connection_t *conn);
static tan_ssl_t tan_recv_header_byte_by_byte(tan_connection_t *conn, char *buf);
static int tan_check_crlf(const char *buf);
static tan_int_t tan_header_parse(tan_connection_t *conn);
static void tan_event_recv_json(tan_connection_t *conn);
static tan_ssl_t tan_recv_json(tan_connection_t *conn);
static tan_int_t tan_packet_handler(tan_connection_t *conn);
static void tan_make_packet(std::string &json_string);
static void tan_send_packet(tan_connection_t *conn);


static const char  *client_errors[] = {
    "invalid packet header received, client: %d.%d.%d.%d",
    "invalid \"user_api\" received, client: %d.%d.%d.%d",
    "invalid \"json_length\" received: %d bytes, "
        "client_max_json_size: %d bytes, client: %d.%d.%d.%d",
    "could not find function: %s, client: %d.%d.%d.%d",
    "%s(): invalid json string received, client: %d.%d.%d.%d",
};


void
tan_event_recv_header(tan_connection_t *conn)
{
    int  ret;

    ret = tan_recv_header(conn);

    switch (ret) {

    case TAN_RECV_HEADER_CONTINUE:

        return;

    case TAN_RECV_HEADER_DONE:

        break;

    case TAN_RECV_HEADER_FAILED:

        goto out_disconnect;
    }

    if (tan_unlikely(conn->status.closing & 1))
        goto out_disconnect;

    if (tan_header_parse(conn) != TAN_OK)
        goto out_disconnect;

    conn->event.json_string.reset(new
        char[conn->event.json_length + 1]());

    tan_event_recv_json(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static int
tan_recv_header(tan_connection_t *conn)
{
    char       buf[2];
    tan_ssl_t  ret;

    tan_memzero(buf, 2);

    for (;;) {

        ret = tan_recv_header_byte_by_byte(conn, buf);

        switch (ret) {

        case TAN_SSL_READ_OK:

            break;

        case TAN_SSL_CONTINUE:

            conn->event.read = tan_event_recv_header;
            return TAN_RECV_HEADER_CONTINUE;

        default:

            return TAN_RECV_HEADER_FAILED;
        }

        conn->event.header.append(buf);

        if (conn->event.header.length() > TAN_MAX_PACKET_HEADER_SIZE)
            return TAN_RECV_HEADER_FAILED;

        if (!tan_check_crlf(conn->event.header.c_str()))
            return TAN_RECV_HEADER_DONE;
    }
}


static tan_ssl_t
tan_recv_header_byte_by_byte(tan_connection_t *conn, char *buf)
{
    int  bytes_read;

    bytes_read = 0;

    return tan_ssl_read(&bytes_read, conn->info.ssl,
                        buf, 1);
}


static int
tan_check_crlf(const char *buf)
{
    int  k;

    for (k = 0; buf[k]; ++k) {

        if (buf[k] == '\r' && buf[k + 1] == '\n')
            return 0;
    }

    return -1;
}


static tan_int_t
tan_header_parse(tan_connection_t *conn)
{
    Json::Value    value;
    const u_char  *p;

    p = tan_get_hostaddr(&conn->info.addr);

    try {
        value = json_decode(conn->event.header);
    } catch (...) {

        tan_log_info(client_errors[TAN_ERR_INVALID_PACKET_HEADER],
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    conn->event.user_api = value["user_api"].asString();
    if (conn->event.user_api.empty()) {

        tan_log_info(client_errors[TAN_ERR_INVALID_USER_API],
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    conn->event.json_length = value["json_length"].asInt();

    if (conn->event.json_length > tan_get_server_cfg()->client_max_json_size ||
        conn->event.json_length <= 0)
    {
        tan_log_info(client_errors[TAN_ERR_INVALID_JSON_LENGTH],
                     conn->event.json_length,
                     tan_get_server_cfg()->client_max_json_size,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    return TAN_OK;
}


static void
tan_event_recv_json(tan_connection_t *conn)
{
    tan_ssl_t  ret;

    ret = tan_recv_json(conn);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_recv_json;
        return;

    default:

        goto out_disconnect;
    }

    if (tan_packet_handler(conn) != TAN_OK)
        goto out_disconnect;

    conn->event.read     = tan_event_recv_header;
    conn->status.closing = 1;

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static tan_ssl_t 
tan_recv_json(tan_connection_t *conn)
{
    return tan_ssl_read(&conn->event.json_read, conn->info.ssl,
                        conn->event.json_string.get(),
                        conn->event.json_length);
}


static tan_int_t
tan_packet_handler(tan_connection_t *conn)
{
    const u_char   *p;
    const char     *func;
    Json::Value     value;
    tan_user_api_t  api;

    p = tan_get_hostaddr(&conn->info.addr);

    func = conn->event.user_api.c_str();

    api = tan_get_user_api_handler(func);
    if (api == NULL) {

        tan_log_info(client_errors[TAN_ERR_FUNCTION_NOT_FOUND],
                     func,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    try {
        value = json_decode(conn->event.json_string.get());
    } catch (...) {

        tan_log_info(client_errors[TAN_ERR_INVALID_JSON_STRING],
                     func,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    tan_set_current_connection(conn);

    try {
        conn->event.packet = api(value);
    } catch (...) {
        return TAN_ERROR;
    }

    tan_make_packet(conn->event.packet);
    tan_send_packet(conn);

    return TAN_OK;
}


static void
tan_make_packet(std::string &json_string)
{
    json_string.append("\r\n");
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

        conn->status.write_pending = 1;
    }
}

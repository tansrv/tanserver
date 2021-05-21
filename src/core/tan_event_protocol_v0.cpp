/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_epoll.h"
#include "tan_events.h"
#include "tan_jsoncpp.h"
#include "tan_user_api.h"


#define TAN_HEADER_SIZE  200


static tan_ssl_t tan_recv_header(tan_connection_t *conn, char *header);
static tan_int_t tan_header_parse(tan_connection_t *conn, const char *header);
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
tan_event_protocol_v0(tan_connection_t *conn)
{
    char       header[TAN_HEADER_SIZE + 1];
    tan_ssl_t  ret;

    tan_memzero(header, TAN_HEADER_SIZE + 1);

    ret = tan_recv_header(conn, header);

    switch (ret) {

    case TAN_SSL_READ_OK:

        break;

    case TAN_SSL_CONTINUE:

        conn->event.read = tan_event_protocol_v0;
        return;

    default:

        goto out_disconnect;
    }

    if (tan_unlikely(conn->status.closing & 1))
        goto out_disconnect;

    if (tan_header_parse(conn, header) != TAN_OK)
        goto out_disconnect;

    conn->event.json_string.reset(new
        char[conn->event.json_length + 1]());

    tan_event_recv_json(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static tan_ssl_t
tan_recv_header(tan_connection_t *conn, char *header)
{
    int  bytes_read;

    bytes_read = 0;

    return tan_ssl_read(&bytes_read, conn->info.ssl,
                        header, TAN_HEADER_SIZE);
}


static tan_int_t
tan_header_parse(tan_connection_t *conn, const char *header)
{
    Json::Value    value;
    const u_char  *p;

    p = tan_get_hostaddr(&conn->info.addr);

    try {
        value = json_decode(header);
    } catch (...) {

        tan_log_info(client_errors[0],
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    conn->event.user_api = value["user_api"].asString();
    if (conn->event.user_api.empty()) {

        tan_log_info(client_errors[1],
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    conn->event.json_length = value["json_length"].asInt();

    if (conn->event.json_length > tan_get_server_cfg()->client_max_json_size ||
        conn->event.json_length <= 0)
    {
        tan_log_info(client_errors[2],
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

    conn->event.read     = tan_event_protocol_v0;
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

        tan_log_info(client_errors[3],
                     func,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    try {
        value = json_decode(conn->event.json_string.get());
    } catch (...) {

        tan_log_info(client_errors[4],
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
    char  buf[101];

    buf[100] = '\0';
    memset(buf, ' ', 100);

    *(buf + snprintf(buf, 101,
                     "{\"json_length\":%u}",
                     (unsigned)json_string.length())) = ' ';

    json_string.insert(0, buf);
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

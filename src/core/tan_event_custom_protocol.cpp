/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_events.h"
#include "tan_jsoncpp.h"
#include "tan_user_api.h"
#include "tan_connection.h"


static tan_int_t tan_header_parse(tan_connection_t *conn);
static void tan_event_recv_json(tan_connection_t *conn);
static tan_ssl_t tan_recv_json(tan_connection_t *conn);
static tan_int_t tan_packet_handler(tan_connection_t *conn);
static void tan_make_packet(std::string &json_string);
static void tan_send_packet(tan_connection_t *conn);


void
tan_event_custom_protocol(tan_connection_t *conn)
{
    if (tan_header_parse(conn) != TAN_OK)
        goto out_disconnect;

    conn->event.content.reset(new
        char[conn->event.content_length + 1]());

    tan_event_recv_json(conn);

    return;

out_disconnect:

    tan_free_client_connection(conn);
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

        tan_log_info(TAN_INVALID_REQUEST_HEADER,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    conn->event.user_api = value["user_api"].asString();
    if (conn->event.user_api.empty()) {

        tan_log_info(TAN_INVALID_USER_API,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    conn->event.content_length = value["json_length"].asInt();

    if (conn->event.content_length > tan_get_server_cfg()->client_max_json_size ||
        conn->event.content_length <= 0)
    {
        tan_log_info(TAN_INVALID_JSON_LENGTH,
                     conn->event.content_length,
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

    conn->event.read     = tan_event_select_protocol;
    conn->status.closing = 1;

    return;

out_disconnect:

    tan_free_client_connection(conn);
}


static tan_ssl_t 
tan_recv_json(tan_connection_t *conn)
{
    return tan_ssl_read(&conn->event.content_read, conn->info.ssl,
                        conn->event.content.get(),
                        conn->event.content_length);
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

        tan_log_info(TAN_FUNCTION_NOT_FOUND,
                     func,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    try {
        value = json_decode(conn->event.content.get());
    } catch (...) {

        tan_log_info(TAN_INVALID_JSON_STRING,
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

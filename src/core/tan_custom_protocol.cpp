/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_python.h"
#include "tan_jsoncpp.h"
#include "tan_connection.h"
#include "tan_custom_protocol.h"


tan_int_t
tan_parse_custom_protocol_header(tan_connection_t *conn,
                                 const char *header)
{
    Json::Value    value;
    const u_char  *p;

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

    conn->event.content_length = value["json_length"].asInt();

    if (conn->event.content_length > tan_get_server_cfg()->client_max_body_size ||
        conn->event.content_length <= 0)
    {
        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_INVALID_JSON_LENGTH,
                     conn->event.content_length,
                     tan_get_server_cfg()->client_max_body_size,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    return TAN_OK;
}


tan_int_t
tan_parse_custom_protocol_body_and_call_api(tan_connection_t *conn,
                                            const char *body)
{
    const char    *func;
    PyObject      *json_obj, *res;
    const u_char  *p;

    p = tan_get_hostaddr(&conn->info.addr);

    func = conn->event.user_api.c_str();

    json_obj = tan_py_json_string_to_object(body);
    if (json_obj == NULL) {

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_INVALID_JSON_STRING,
                     func,
                     p[0], p[1], p[2], p[3]);

        return TAN_ERROR;
    }

    /*
     * This allows us to call tan_get_current_connection()
     * in pg_query() to get connection information.
     */
    tan_set_current_connection(conn);

    res = tan_call_user_api(func, json_obj);
    if (res == NULL) {

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_FUNCTION_NOT_FOUND,
                     func,
                     p[0], p[1], p[2], p[3]);

        Py_DECREF(json_obj);
        return TAN_ERROR;
    }

    conn->event.packet = PyUnicode_AsUTF8(res);

    Py_DECREF(json_obj);
    Py_DECREF(res);

    return TAN_OK;
}

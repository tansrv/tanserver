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


#define TAN_WS_GET_PAYLOAD_LEN_7(buf)                 \
    (buf[0] & 0x7f)

#define TAN_WS_GET_PAYLOAD_LEN_7_16(buf)              \
    ((buf[0] & 0xff << 8) | (buf[1] & 0xff))

#define TAN_WS_GET_PAYLOAD_LEN_7_64(buf)              \
    ((buf[4] & 0xff << 24) | (buf[5] & 0xff << 16) |  \
        (buf[6] & 0xff << 8) | (buf[7] & 0xff))


typedef enum {
    TAN_WS_READ_HEADER_FAILED = 0,
    TAN_WS_READ_HEADER_CONTINUE,
    TAN_WS_READ_HEADER_DONE,
} tan_ws_read_header_status_e;


void
tan_event_websocket(tan_connection_t *conn)
{

}

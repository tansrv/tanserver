/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_websocket.h"


#define TAN_WEBSOCKET_GUID  "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"


char *
tan_generate_sec_websocket_accept(const char *sec_websocket_key)
{
    char  buf[TAN_MAX_STR_SIZE], sha1_res[TAN_SHA1_LENGTH + 1], *base64_res;

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    snprintf(buf, TAN_MAX_STR_SIZE, "%s%s",
             sec_websocket_key, TAN_WEBSOCKET_GUID);

    tan_memzero(sha1_res, TAN_SHA1_LENGTH + 1);

    tan_sha1_encode((const unsigned char *)buf,
                    strlen(buf), (unsigned char *)sha1_res);

    return tan_base64_encode(sha1_res, TAN_SHA1_LENGTH);
}


std::string
tan_ws_make_packet(const char *buf)
{
    int          len;
    char         header[11];
    std::string  packet;

    tan_memzero(header, 11);

    len = strlen(buf);

    header[0] = 0x81;

    if (len < 126) {
        header[1] = len;

    } else if (len < 65536) {
        header[1] = 126;

        header[2] = len >> 8 & 0xff;
        header[3] = len & 0xff;

    } else {
        header[1] = 127;

        header[2] = 0;
        header[3] = 0;
        header[4] = 0;
        header[5] = 0;

        header[6] = len >> 24 & 0xff;
        header[7] = len >> 16 & 0xff;
        header[8] = len >> 8 & 0xff;
        header[9] = len & 0xff;
    }

    packet.append(header);
    packet.append(buf);

    return packet;
}

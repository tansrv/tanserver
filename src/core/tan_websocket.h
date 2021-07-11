/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_WEBSOCKET_H
#define TAN_WEBSOCKET_H


#include "tan_core.h"

#include <iostream>


#define TAN_WEBSOCKET_RESPONSE_HEADER  "HTTP/1.1 101 Switching Protocols\r\n"  \
                                       "Upgrade: websocket\r\n"                \
                                       "Connection: Upgrade\r\n"               \
                                       "Sec-WebSocket-Accept: %s\r\n\r\n"

/**
 * Generate Sec-WebSocket-Accept for WebSocket handshake.
 *
 * @param sec_websocket_key: Client's Sec-WebSocket-Key
 *
 * @return Sec-WebSocket-Accept
 *
 * @note Caller is responsible for freeing the returned buffer.
 */
char *tan_generate_sec_websocket_accept(const char *sec_websocket_key);

/**
 * Generate a WebSocket frame.
 *
 * @param buf: Data to be framed
 *
 * @return A WebSocket frame
 */
std::string tan_ws_make_packet(const char *buf);


#endif /* TAN_WEBSOCKET_H */

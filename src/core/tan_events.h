/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_EVENTS_H
#define TAN_EVENTS_H


#include "tan_core.h"
#include "tan_connection.h"


void tan_event_loop(tan_socket_t server);

void tan_event_accept(tan_connection_t *conn);
void tan_event_ssl_handshake(tan_connection_t *conn);

void tan_event_select_protocol(tan_connection_t *conn);
void tan_event_custom_protocol(tan_connection_t *conn);
void tan_event_websocket(tan_connection_t *conn);

void tan_event_write(tan_connection_t *conn);


#endif /* TAN_EVENTS_H */

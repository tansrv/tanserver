/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_CUSTOM_PROTOCOL_H
#define TAN_CUSTOM_PROTOCOL_H


#include "tan_core.h"
#include "tan_connection.h"

#include <string>


/**
 * Parse the custom protocol header and store all 
 * values in tan_connection_t ("user_api", "json_length").
 *
 * @param conn:   Client connection pointer
 * @param header: Custom protocol header, like: {"user_api":"api","json_length":2}\r\n
 *
 * @return TAN_OK || TAN_ERROR
 */
tan_int_t tan_parse_custom_protocol_header(tan_connection_t *conn,
                                           const char *header);

/**
 * Parse the custom protocol body and call the API according 
 * to conn->event.user_api. The returned string 
 * will be stored in conn->event.packet.
 *
 * @param conn: Client connection pointer
 * @param body: Custom protocol body (A JSON string)
 *
 * @return TAN_OK || TAN_ERROR
 */
tan_int_t tan_parse_custom_protocol_body_and_call_api(tan_connection_t *conn,
                                                      const char *body);


#endif /* TAN_CUSTOM_PROTOCOL_H */

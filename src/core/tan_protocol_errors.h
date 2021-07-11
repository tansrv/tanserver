/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_PROTOCOL_ERRORS_H
#define TAN_PROTOCOL_ERRORS_H


#define TAN_INVALID_PACKET_HEADER                   \
    "invalid packet header received, client: %d.%d.%d.%d"

#define TAN_INVALID_USER_API                        \
    "invalid \"user_api\" received, client: %d.%d.%d.%d"

#define TAN_INVALID_JSON_LENGTH                     \
    "invalid \"json_length\" received: %d bytes, "  \
        "client_max_json_size: %d bytes, client: %d.%d.%d.%d"

#define TAN_FUNCTION_NOT_FOUND                      \
    "could not find function: %s, client: %d.%d.%d.%d"

#define TAN_INVALID_JSON_STRING                     \
    "%s(): invalid json string received, client: %d.%d.%d.%d"


#endif /* TAN_PROTOCOL_ERRORS_H */

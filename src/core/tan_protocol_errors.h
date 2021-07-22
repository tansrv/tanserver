/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_PROTOCOL_ERRORS_H
#define TAN_PROTOCOL_ERRORS_H


#define TAN_PROTOCOL_COMMON_ERROR  "common: "
#define TAN_WEBSOCKET_ERROR        "websocket: "
#define TAN_CUSTOM_PROTOCOL_ERROR  "custom protocol: "
#define TAN_CLIENT_HOSTADDR        ", client: %d.%d.%d.%d"


#define TAN_PROTOCOL_COMMON_ERROR_REQUEST_HEADER_TOO_LARGE  \
    TAN_PROTOCOL_COMMON_ERROR                               \
        "request header too large"                          \
    TAN_CLIENT_HOSTADDR

#define TAN_CUSTOM_PROTOCOL_ERROR_INVALID_REQUEST_HEADER    \
    TAN_CUSTOM_PROTOCOL_ERROR                               \
        "invalid request header received"                   \
    TAN_CLIENT_HOSTADDR

#define TAN_CUSTOM_PROTOCOL_ERROR_REQUEST_HEADER_TOO_LARGE  \
    TAN_CUSTOM_PROTOCOL_ERROR                               \
        "request header too large"                          \
    TAN_CLIENT_HOSTADDR

#define TAN_WEBSOCKET_ERROR_REQUEST_HEADER_TOO_LARGE        \
    TAN_WEBSOCKET_ERROR                                     \
        "request header too large"                          \
    TAN_CLIENT_HOSTADDR

#define TAN_WEBSOCKET_ERROR_SEC_WEBSOCKET_KEY_NOT_FOUND     \
    TAN_WEBSOCKET_ERROR                                     \
        "\"Sec-WebSocket-Key\" not found"                   \
    TAN_CLIENT_HOSTADDR

#define TAN_WEBSOCKET_ERROR_PAYLOAD_LENGTH_TOO_LARGE        \
    TAN_WEBSOCKET_ERROR                                     \
        "payload length too large: %d bytes, "              \
            "client_max_body_size: %d bytes"                \
    TAN_CLIENT_HOSTADDR

#define TAN_CUSTOM_PROTOCOL_ERROR_USER_API_NOT_FOUND        \
    TAN_CUSTOM_PROTOCOL_ERROR                               \
        "\"user_api\" not found"                            \
    TAN_CLIENT_HOSTADDR

#define TAN_CUSTOM_PROTOCOL_ERROR_INVALID_JSON_LENGTH       \
    TAN_CUSTOM_PROTOCOL_ERROR                               \
        "invalid \"json_length\" received: %d bytes, "      \
            "client_max_body_size: %d bytes"                \
    TAN_CLIENT_HOSTADDR

#define TAN_CUSTOM_PROTOCOL_ERROR_FUNCTION_NOT_FOUND        \
    TAN_CUSTOM_PROTOCOL_ERROR                               \
        "could not find function: %s"                       \
    TAN_CLIENT_HOSTADDR

#define TAN_CUSTOM_PROTOCOL_ERROR_INVALID_JSON_STRING       \
    TAN_CUSTOM_PROTOCOL_ERROR                               \
        "%s(): invalid json string received"                \
    TAN_CLIENT_HOSTADDR


#endif /* TAN_PROTOCOL_ERRORS_H */

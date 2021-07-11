/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_CONNECTION_H
#define TAN_CONNECTION_H


#include "tan_core.h"
#include "tan_openssl_ssl.h"

#include <string>
#include <memory>


typedef struct tan_connection_s  tan_connection_t;


typedef struct {
    SSL                     *ssl;
    tan_socket_t             fd;

    struct sockaddr_in       addr;
    socklen_t                addrlen;

    char                     protocol;

    struct timeval           start;
} tan_conn_info_t;


typedef struct {
    unsigned                 events;

    void                   (*read)(tan_connection_t *conn);

    std::string              header;

    std::string              user_api;
    int                      json_length;

    std::unique_ptr<char[]>  json_string;
    int                      json_read;

    void                   (*write)(tan_connection_t *conn);
    std::string              packet;
} tan_conn_event_t;


typedef struct {
    unsigned                 seed;
    unsigned                 write_pending : 1;
    unsigned                 closing : 1;
    unsigned                 instance : 1;
} tan_conn_status_t;


struct tan_connection_s {
    tan_conn_info_t          info;
    tan_conn_event_t         event;
    tan_conn_status_t        status;
    tan_list_node_t          node;
};


tan_int_t tan_idle_connections_init();
tan_list_node_t *tan_get_idle_connections();
void tan_set_current_connection(tan_connection_t *conn);
tan_connection_t *tan_get_current_connection();
void tan_write_access_log(tan_connection_t *conn);
void tan_free_client_connection(tan_connection_t *conn);


#endif /* TAN_CONNECTION_H */

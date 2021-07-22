/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_EPOLL_H
#define TAN_EPOLL_H


#include "tan_core.h"
#include "tan_connection.h"


tan_int_t tan_epoll_init();
tan_int_t tan_epoll_ctl_add(unsigned events, tan_connection_t *conn);
tan_int_t tan_add_listen_socket_to_epoll(tan_socket_t fd);
void tan_epoll_process_events();


#endif /* TAN_EPOLL_H */

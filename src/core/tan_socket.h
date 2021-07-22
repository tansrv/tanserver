/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_SOCKET_H
#define TAN_SOCKET_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


typedef int  tan_socket_t;


tan_int_t tan_create_listen_socket(tan_socket_t *fd, in_port_t port);
const u_char *tan_get_hostaddr(struct sockaddr_in *addr);


#define tan_close_socket(fd)  close(fd)


#ifdef __cplusplus
}
#endif


#endif /* TAN_SOCKET_H */

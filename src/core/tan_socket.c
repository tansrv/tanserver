/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


#define TAN_BACKLOG  1024


static tan_int_t tan_socket_disable_nagle(tan_socket_t fd);
static tan_int_t tan_socket_allow_reuseport(tan_socket_t fd);


tan_int_t
tan_create_listen_socket(tan_socket_t *fd, in_port_t port)
{
    tan_int_t           ret;
    struct sockaddr_in  addr;

    *fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (*fd == -1) {

        tan_log_crit(errno, "socket() failed", NULL);
        return TAN_ERROR;
    }

    ret = tan_socket_disable_nagle(*fd);
    if (ret != TAN_OK)
        goto failed;

    ret = tan_socket_allow_reuseport(*fd);
    if (ret != TAN_OK)
        goto failed;

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(*fd, (struct sockaddr *)&addr, sizeof(addr))) {

        tan_log_crit(errno, "bind() failed", NULL);
        goto failed;
    }

    if (listen(*fd, TAN_BACKLOG)) {

        tan_log_crit(errno, "listen() failed", NULL);
        goto failed;
    }

    return TAN_OK;

failed:

    tan_close_socket(*fd);
    return TAN_ERROR;
}


static tan_int_t
tan_socket_disable_nagle(tan_socket_t fd)
{
    int  on;

    on = 1;

    if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                   (const char *)&on, sizeof(int)))
    {
        tan_log_warn(errno, "setsockopt(TCP_NODELAY) failed", NULL);
        return TAN_ERROR;
    }

    return TAN_OK;
}


/* Avoid thundering herd.  */
static tan_int_t
tan_socket_allow_reuseport(tan_socket_t fd)
{
    int  on;

    on = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT,
                   (const char *)&on, sizeof(int)))
    {
        tan_log_warn(errno, "setsockopt(SO_REUSEPORT) failed", NULL);
        return TAN_ERROR;
    }

    return TAN_OK;
}


const u_char *
tan_get_hostaddr(struct sockaddr_in *addr)
{
    return (const u_char *)&addr->sin_addr;
}

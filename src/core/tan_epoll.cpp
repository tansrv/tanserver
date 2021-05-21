/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_epoll.h"
#include "tan_events.h"


#define TAN_MAXEVENTS  1024


static struct {
    tan_fd_t            fd;
    struct epoll_event  events[TAN_MAXEVENTS];
} epoll;


tan_int_t
tan_epoll_init()
{
    epoll.fd = epoll_create1(0);
    if (epoll.fd == -1) {

        tan_log_crit(errno, "epoll_create1() failed", NULL);
        return TAN_ERROR;
    }

    return TAN_OK;
}


tan_int_t
tan_epoll_ctl_add(unsigned events, tan_connection_t *conn)
{
    struct epoll_event  ee;

    ee.events   = events;
    ee.data.ptr = (void *)((uintptr_t)conn | conn->status.instance);

    conn->event.events = events;

    if (epoll_ctl(epoll.fd, EPOLL_CTL_ADD, conn->info.fd, &ee)) {

        tan_log_crit(errno, "epoll_ctl() failed", NULL);
        return TAN_ERROR;
    }

    return TAN_OK;
}


tan_int_t
tan_add_listen_socket_to_epoll(tan_socket_t fd)
{
    tan_connection_t  *conn;

    conn = (tan_connection_t *)malloc(sizeof(tan_connection_t));
    if (conn == NULL) {

        tan_log_crit(errno, "malloc() failed", NULL);
        return TAN_ERROR;
    }

    conn->info.fd         = fd;
    conn->event.read      = tan_event_accept;
    conn->status.instance = 0;

    return tan_epoll_ctl_add(EPOLLIN, conn);
}


void
tan_epoll_process_events()
{
    int                k, nfds;
    unsigned           ev, instance;
    void              *ptr;
    tan_connection_t  *conn;

    nfds = epoll_wait(epoll.fd, epoll.events, TAN_MAXEVENTS, 100);
    if (nfds == -1) {

        if (errno != EINTR)
            tan_log_crit(errno, "epoll_wait() failed", NULL);
    }

    for (k = 0; k < nfds; ++k) {

        ptr = epoll.events[k].data.ptr;

        instance = (uintptr_t)ptr & 1;

        conn = (tan_connection_t *)((uintptr_t)ptr &~ 1);

        /* The way to deal with stale events is the same as Nginx  */
        if (conn->info.fd == -1 ||
            conn->status.instance != instance)
        {
            continue;
        }

        ev = epoll.events[k].events;

        if (ev & EPOLLIN)
            conn->event.read(conn);
        else if (ev & EPOLLOUT)
            conn->event.write(conn);
    }
}

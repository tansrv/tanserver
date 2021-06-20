/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_epoll.h"
#include "tan_events.h"


static tan_int_t tan_client_connection_init(tan_connection_t *conn,
                                            tan_connection_t *client);
static tan_int_t tan_accept_new_socket(tan_connection_t *conn,
                                       tan_connection_t *client);
static tan_int_t tan_try_ssl_handshake(tan_connection_t *client);
static tan_int_t tan_add_new_socket_to_epoll(tan_connection_t *client);
static void tan_check_request_timeout(void *data, unsigned u32);


void
tan_event_accept(tan_connection_t *conn)
{
    tan_connection_t  *client;

    for (;;) {

        if (tan_unlikely(tan_list_empty(tan_get_idle_connections()))) {

            tan_log_info("running out of idle connections, "
                         "open files limit: %d",
                         tan_get_open_files_limit());

            return;
        }

        client = tan_list_get_data(tan_list_first(tan_get_idle_connections()),
                                   tan_connection_t, node);

        if (tan_client_connection_init(conn, client) != TAN_OK)
            return;

        tan_list_remove(&client->node);

        tan_active_connections_atomic_inc();
    }
}


static tan_int_t
tan_client_connection_init(tan_connection_t *conn,
                           tan_connection_t *client)
{
    tan_int_t  ret;

    for (;;) {

        ret = tan_accept_new_socket(conn, client);
        if (ret != TAN_OK)
            return ret;

        client->info.ssl = tan_ssl_new(client->info.fd);
        if (client->info.ssl == NULL)
            goto failed;

        ret = tan_try_ssl_handshake(client);
        if (ret != TAN_OK)
            goto failed;

        ret = tan_add_new_socket_to_epoll(client);
        if (ret != TAN_OK)
            goto failed;

        /*
         * We need to add a timed task. If the client does not
         * complete the request within request_timeout (seconds),
         * we will disconnect from the client.
         */
        ret = tan_event_add_timer(tan_check_request_timeout,
                                  client, client->status.seed,
                                  tan_get_server_cfg()->request_timeout);

        if (ret == TAN_OK)
            return ret;

    failed:

        tan_ssl_close(&client->info.ssl, &client->info.fd);
    }
}


static tan_int_t
tan_accept_new_socket(tan_connection_t *conn,
                      tan_connection_t *client)
{
    for (;;) {

        client->info.fd = accept4(conn->info.fd,
                                  (struct sockaddr *)&client->info.addr,
                                  &client->info.addrlen,
                                  SOCK_NONBLOCK);

        if (client->info.fd != -1) {

            /* In order to get request time and write it to the access log.  */
            gettimeofday(&client->info.start, NULL);
            return TAN_OK;
        }

        if (errno == EAGAIN) /* not an error  */
            return TAN_ERROR;
        else if (errno == EINTR || errno == ECONNABORTED)
            continue;

        tan_log_crit(errno, "accept4() failed", NULL);
        return TAN_ERROR;
    }
}


static tan_int_t
tan_try_ssl_handshake(tan_connection_t *client)
{
    tan_ssl_t  ret;

    ret = tan_ssl_accept(client->info.ssl);

    switch (ret) {

    case TAN_SSL_CONTINUE:

        client->event.read = tan_event_ssl_handshake;
        return TAN_OK;

    /* unlikely  */
    case TAN_SSL_ACCEPT_OK:

        client->event.read = tan_event_recv_header;
        return TAN_OK;

    default:

        return TAN_ERROR;
    }
}


static tan_int_t
tan_add_new_socket_to_epoll(tan_connection_t *client)
{
    /* listen socket: LT mode, client socket: ET mode  */
    return tan_epoll_ctl_add(EPOLLOUT | EPOLLIN | EPOLLET, client);
}


static void
tan_check_request_timeout(void *data, unsigned u32)
{
    tan_connection_t  *conn;

    conn = (tan_connection_t *)data;

    /*
     * If u32 == conn->status.seed, it means that
     * the client does not complete the request within
     * request_timeout (seconds), and we should disconnect from it.
     *
     * Note: conn->status.seed is a random value,
     *       this value will be reset when the connection is released.
     */
    if (u32 == conn->status.seed && conn->info.fd != -1)
        tan_free_client_connection(conn);
}

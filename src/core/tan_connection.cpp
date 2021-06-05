/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_events.h"
#include "tan_connection.h"


static void tan_single_connection_init(tan_connection_t *conn);
static unsigned tan_get_request_time(tan_connection_t *conn);


static tan_list_node_t    idle_connections;
static tan_connection_t  *current_connection;


tan_int_t
tan_idle_connections_init()
{
    int                k;
    void              *mem;
    tan_connection_t  *conn;

    srand(time(NULL));

    mem = calloc(tan_get_open_files_limit(),
                 sizeof(tan_connection_t));

    if (mem == NULL) {

        tan_log_crit(errno, "calloc() failed", NULL);
        return TAN_ERROR;
    }

    tan_list_init(&idle_connections);

    for (k = 0; k < tan_get_open_files_limit(); ++k) {

        conn = new((char *)mem +
                   (sizeof(tan_connection_t) * k)) tan_connection_t();

        tan_single_connection_init(conn);

        tan_list_push_back(&idle_connections, &conn->node);
    }

    return TAN_OK;
}


static void
tan_single_connection_init(tan_connection_t *conn)
{
    conn->info.ssl             = NULL;
    conn->info.fd              = -1;
    conn->info.addrlen         = sizeof(struct sockaddr_in);

    conn->event.read           = NULL;
    conn->event.write          = tan_event_write;
    conn->event.json_read      = 0;

    conn->status.seed          = rand();
    conn->status.closing       = 0;
    conn->status.instance      = 0;
    conn->status.write_pending = 0;
}


tan_list_node_t *
tan_get_idle_connections()
{
    return &idle_connections;
}


void
tan_set_current_connection(tan_connection_t *conn)
{
    current_connection = conn;
}


tan_connection_t *
tan_get_current_connection()
{
    return current_connection;
}


void
tan_write_access_log(tan_connection_t *conn)
{
    char           buf[TAN_MAX_STR_SIZE];
    const u_char  *p;

    p = tan_get_hostaddr(&conn->info.addr);

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    snprintf(buf, TAN_MAX_STR_SIZE,
             "[%d.%d.%d.%d] [%s] "
             "accessed %s(), request time: %u ms\n",
             p[0], p[1], p[2], p[3],
             tan_get_localtime_str(),
             conn->event.user_api.c_str(),
             tan_get_request_time(conn));

    if (write(tan_get_access_log(), buf, strlen(buf)) == -1)
        tan_stderr_info(buf);
}


static unsigned
tan_get_request_time(tan_connection_t *conn)
{
    struct timeval  end;

    gettimeofday(&end, NULL);

    return (end.tv_sec * 1000 + end.tv_usec / 1000) -
            (conn->info.start.tv_sec * 1000 +
            conn->info.start.tv_usec / 1000);
}


void
tan_free_client_connection(tan_connection_t *conn)
{
    SSL_shutdown(conn->info.ssl);
    tan_ssl_close(&conn->info.ssl, &conn->info.fd);

    conn->event.header.clear();

    conn->event.read           = NULL;
    conn->event.json_read      = 0;

    conn->status.seed          = rand_r(&conn->status.seed);
    conn->status.closing       = 0;
    conn->status.instance     =~ conn->status.instance;
    conn->status.write_pending = 0;

    tan_list_push_back(&idle_connections, &conn->node);

    tan_active_connections_atomic_dec();
}

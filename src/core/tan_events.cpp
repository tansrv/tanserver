/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_epoll.h"
#include "tan_events.h"


static void tan_process_events_and_timers();
static void tan_set_worker_exit(void *data, unsigned u32);


static int  worker_exit;


void
tan_event_loop(tan_socket_t server)
{
    while (!tan_is_stop())
        tan_process_events_and_timers();

    /* Close listen socket.  */
    tan_close_socket(server);

    /*
     * When the exit signal (SIGUSR1) is received,
     * the worker process will exit after request_timeout (seconds)
     * to ensure that all events are processed.
     */
    if (tan_event_add_timer(tan_set_worker_exit, NULL, 0,
                            tan_get_server_cfg()->request_timeout)
        != TAN_OK)
    {
        return;
    }

    while (!worker_exit)
        tan_process_events_and_timers();
}


static void
tan_process_events_and_timers()
{
    tan_time_update();

    if (tan_unlikely(tan_start_reload())) {

        tan_reload_user_api();

        tan_reload_end();
    }

    tan_event_expire_timers();

    tan_epoll_process_events();
}


static void
tan_set_worker_exit(void *data, unsigned u32)
{
    worker_exit = 1;
}

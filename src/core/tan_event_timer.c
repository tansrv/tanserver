/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


typedef struct {
    time_t           expires;
    void            *data;
    unsigned         u32;
    void           (*handler)(void *data, unsigned u32);
    tan_list_node_t  node;
} tan_timer_t;


static tan_list_node_t  timers;


void
tan_event_timer_init()
{
    tan_list_init(&timers);
}


tan_int_t
tan_event_add_timer(void (*handler)(void *data, unsigned u32),
                    void *data, unsigned u32, time_t sec)
{
    tan_timer_t  *timer;

    timer = (tan_timer_t *)malloc(sizeof(tan_timer_t));
    if (timer == NULL) {

        tan_log_crit(errno, "malloc() failed", NULL);
        return TAN_ERROR;
    }

    timer->data    = data;
    timer->u32     = u32;
    timer->handler = handler;
    timer->expires = tan_get_time() + sec;

    tan_list_push_back(&timers, &timer->node);
    return TAN_OK;
}


void
tan_event_expire_timers()
{
    tan_timer_t      *timer;
    tan_list_node_t  *tmp;

    tan_list_for_each(&timers, timer, tmp, tan_timer_t, node) {
 
        if (tan_get_time() >= timer->expires) {

            tan_list_remove(&timer->node);

            timer->handler(timer->data, timer->u32);
            free(timer);
        }
    }
}

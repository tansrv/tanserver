/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_event_timer.h"


typedef struct {
    time_t    expires;
    void     *data;
    unsigned  u32;
    void    (*handler)(void *data, unsigned u32);
} tan_timer_t;


static tan_heap_t  *timers;


tan_int_t
tan_event_timer_init()
{
    timers = tan_heap_create();
    if (timers == NULL)
        return TAN_ERROR;

    return TAN_OK;
}


tan_int_t
tan_event_add_timer(void (*handler)(void *data, unsigned u32),
                    void *data, unsigned u32, time_t sec)
{
    tan_int_t     ret;
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

    ret = tan_heap_add_node(timers, tan_get_time() + sec, timer);
    if (ret != TAN_OK) {

        free(timer);
        return ret;
    }

    return TAN_OK;
}


void
tan_event_expire_timers()
{
    tan_timer_t  *timer;

    for (;;) {

        if (!timers->used)
            return;

        timer = tan_heap_min_peek(timers);
        if (tan_get_time() < timer->expires)
            return;

        timer = tan_heap_remove_min(timers);

        timer->handler(timer->data, timer->u32);
        free(timer);
    }
}

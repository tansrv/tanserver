/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_EVENT_TIMER_H
#define TAN_EVENT_TIMER_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


void tan_event_timer_init();
tan_int_t tan_event_add_timer(void (*handler)(void *data, unsigned u32),
                              void *data, unsigned u32, time_t sec);
void tan_event_expire_timers();


#ifdef __cplusplus
}
#endif


#endif /* TAN_EVENT_TIMER_H */

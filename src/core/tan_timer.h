/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_TIMER_H
#define TAN_TIMER_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


void tan_timer_init();
tan_int_t tan_add_timer(void (*handler)(void *data, unsigned u32),
                        void *data, unsigned u32, time_t sec);
void tan_expire_timers();


#ifdef __cplusplus
}
#endif


#endif /* TAN_TIMER_H */

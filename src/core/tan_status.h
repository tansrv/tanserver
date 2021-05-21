/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_STATUS_H
#define TAN_STATUS_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


tan_int_t tan_status_shm_init();
tan_int_t tan_status_shm_attach(int prot, int flags);
int tan_is_running();
time_t tan_get_uptime();
volatile unsigned tan_get_active_connections();
volatile unsigned long long tan_get_handled_requests();
void tan_active_connections_atomic_inc();
void tan_active_connections_atomic_dec();
void tan_handled_requests_atomic_inc();
void tan_status_shm_munmap();
void tan_status_shm_free();


#ifdef __cplusplus
}
#endif


#endif /* TAN_STATUS_H */

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_PROCESS_H
#define TAN_PROCESS_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


tan_int_t tan_daemon();
tan_int_t tan_create_pidfile();
void tan_remove_pidfile();
void tan_set_process_name(const char *name);


#ifdef __cplusplus
}
#endif


#endif /* TAN_PROCESS_H */

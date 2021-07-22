/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_SIGNAL_H
#define TAN_SIGNAL_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


tan_int_t tan_signal_init();
int tan_is_stop();
int tan_start_reload();
void tan_reload_end();


#ifdef __cplusplus
}
#endif


#endif /* TAN_SIGNAL_H */

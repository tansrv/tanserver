/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_SYSCFG_H
#define TAN_SYSCFG_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


tan_int_t tan_syscfg_init();
int tan_get_core_count();
int tan_get_page_size();
int tan_get_open_files_limit();


#ifdef __cplusplus
}
#endif


#endif /* TAN_SYSCFG_H */

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_AFFINITY_H
#define TAN_AFFINITY_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


void tan_setaffinity(int core);


#ifdef __cplusplus
}
#endif


#endif /* TAN_AFFINITY_H */

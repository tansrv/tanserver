/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_SHM_H
#define TAN_SHM_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


typedef void  tan_shm_t;


tan_shm_t *tan_shm_create(const char *name, size_t size,
                          int prot, int flags);
tan_shm_t *tan_shm_attach(const char *name, size_t size,
                          int prot, int flags);
void tan_munmap(tan_shm_t *shm, size_t size);
void tan_shm_free(tan_shm_t *shm, size_t size, const char *name);


#ifdef __cplusplus
}
#endif


#endif /* TAN_SHM_H */

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 */


#ifndef TAN_HEAP_H
#define TAN_HEAP_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


/*
 * Usage example:
 *
 * char a = 'a';
 * char b = 'b';
 * char c = 'c';
 * char d = 'd';
 *
 * tan_heap_t *th = tan_heap_create(1, &a);
 *
 * tan_heap_add_node(th, 2, &b);
 * tan_heap_add_node(th, 3, &c);
 * tan_heap_add_node(th, 4, &d);
 *
 * tan_heap_print(th);
 *
 * char *get = tan_heap_remove_min(th);
 *
 * printf("%c\n", *get);
 *
 * tan_heap_print(th);
 */


typedef struct {
    time_t             key;
    void              *value;
} tan_heap_node_t;


typedef struct {
    size_t             size;
    size_t             used;
    tan_heap_node_t  **arr;
} tan_heap_t;


tan_heap_t *tan_heap_create(time_t key, void *value);
tan_int_t tan_heap_add_node(tan_heap_t *hh, time_t key, void *value);
void *tan_heap_min_peek(tan_heap_t *hh);
void *tan_heap_remove_min(tan_heap_t *hh);
void tan_heap_destory(tan_heap_t *hh);

#if 0
void tan_heap_print(tan_heap_t *hh);
#endif


#ifdef __cplusplus
}
#endif


#endif /* TAN_HEAP_H */

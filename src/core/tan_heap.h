/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_HEAP_H
#define TAN__H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"

#include <stdlib.h>
#include <time.h>
#include <string.h>

#define INIT_SIZE 20

struct tan_node
{
    time_t key;
    void   *value;
};

typedef struct tan_node Tnode;

struct tan_heap
{
    Tnode** arr;
    size_t size;
    size_t used;
};

typedef struct tan_heap Theap;

Theap* createHeap(time_t, void*);
Tnode* minPeek(Theap*);
void addNode(Theap*, time_t, void*);
void printHeap(Theap*);


#ifdef __cplusplus
}
#endif


#endif /* TAN_HEAP_H */

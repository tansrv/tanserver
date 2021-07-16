/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_HEAP_H
#define TAN_HEAP_H


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

/*
    Usage example:
    
    char a = 'a';
    char b = 'b';
    char c = 'c';
    char d = 'd';
    Theap* th = createHeap(1, &a);
    addNode(th, 2, &b);
    addNode(th, 3, &c);
    addNode(th, 4, &d);

    printHeap(th);

    char* get = removeMin(th);

	printf("%c\n", *get);
	
    printHeap(th);
*/

Theap* createHeap(time_t, void*);
void* minPeek(Theap*);
void* removeMin(Theap*);
void* removeNode(Theap*, time_t);
void addNode(Theap*, time_t, void*);
void printHeap(Theap*);

#ifdef __cplusplus
}
#endif


#endif /* TAN_HEAP_H */

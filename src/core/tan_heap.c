/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */

#include "tan_heap.h"

static void createNode(Theap *hp,
                       time_t key,
                       void *value)
{
    if(hp->used >= hp->size)
    {
        Tnode** hhNew = (Tnode**) calloc(2*hp->size, sizeof(Tnode*));
        //if array is full double its dimension
        for(int i = 0; i<hp->size; i++)
            hhNew[i] = hp->arr[i];

        free(hp->arr);
        hp->arr = hhNew;
        hp->size = 2*hp->size;
    }
    hp->arr[hp->used] = (Tnode*) malloc(sizeof(Tnode));
    hp->arr[hp->used]->key = key;
    hp->arr[hp->used]->value = value;
    hp->used++;
}

static time_t getKey(Tnode *n)
{
    return n->key;
}

static void* getValue(Tnode *n)
{
    return n->value;
}

static int getParentIndex(int i)
{
    return (i-2+(i&1))>>1;
}

Theap* createHeap(time_t key, 
                  void *value)
{
    //initial size: 20
    Theap* hh = (Theap*) malloc(sizeof(Theap));
    hh->arr = (Tnode**) calloc(INIT_SIZE, sizeof(Tnode*));

    hh->size = INIT_SIZE;
    hh->used = 0;

    createNode(hh, key, value);

    return hh;
}

Tnode* minPeek(Theap *hh)
{
    return hh->arr[0];
}

Tnode* removeMin()
{

}

void addNode(Theap *hh,
             time_t key, 
             void* value)
{
    createNode(hh, key, value);
    
    //upheap
    int c = hh->used-1;
    int par;
    Tnode* tmp;
    while(c > 0){
        par = getParentIndex(c);
        if(getKey(hh->arr[c]) >= getKey(hh->arr[par]))
            break;
        //swap
        tmp = hh->arr[c];
        hh->arr[c] = hh->arr[par];
        hh->arr[par] = tmp;
        c = par;
    }
}

void printHeap(Theap *hh){
    printf("[");
    for(int i = 0; i<hh->used; i++)
        printf("%lu ", hh->arr[i]->key);
    
    printf("]\n");
}
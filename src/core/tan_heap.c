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

static int getLeftIndex(int i)
{
    return (2*i) + 1;
}

static int getRightIndex(int i)
{
    return (2*i) + 2;
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

void* minPeek(Theap *hh)
{
    return hh->arr[0]->value;
}

void* removeMin(Theap *hh)
{
    if(hh->used == 0)
        return NULL;

    void* out = minPeek(hh);
    free(hh->arr[0]);

    //swap rightmost leaf and root 
    hh->arr[0] = hh->arr[hh->used-1];
    hh->arr[hh->used-1] = NULL;
    hh->used--;

    //downheap
    int    c = 0;
    int    l;
    int    r;
    Tnode* tmp;
    while(c < hh->used)
    {
        l = getLeftIndex(c);
        r = getRightIndex(c);
        if(r < hh->used && l < hh->used && hh->arr[l]->key < hh->arr[r]->key)
        {
            //left is smaller
            if(hh->arr[c]->key > hh->arr[l]->key){
                //swap arr[c] and left child
                tmp = hh->arr[c];
                hh->arr[c] = hh->arr[l];
                hh->arr[l] = tmp;
                c = l;
            }
            else
                break;
        }
        else
        {
            //right is smaller
            if(r < hh->used && hh->arr[c]->key > hh->arr[r]->key)
            {
                //swap arr[c] and right child
                tmp = hh->arr[c];
                hh->arr[c] = hh->arr[r];
                hh->arr[r] = tmp;
                c = r;
            }
            else
                break;
        }
    }

    return out;
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
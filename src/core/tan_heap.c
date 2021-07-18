/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_heap.h"


#define TAN_HEAP_INIT_SIZE  20


static tan_int_t tan_heap_create_node(tan_heap_t *hp, time_t key, void *value);
static int tan_heap_get_parent_index(int i);
static time_t tan_heap_get_key(tan_heap_node_t *n);
static void* tan_heap_get_value(tan_heap_node_t *n);
static int tan_heap_get_left_index(int i);
static int tan_heap_get_right_index(int i);
static void tan_downheap(tan_heap_t *hh, int index);
static int tan_findNodeByValue(tan_heap_t *hh, int index, void *value);

tan_heap_t *
tan_heap_create()
{
    tan_heap_t  *hh;

    hh = (tan_heap_t *)malloc(sizeof(tan_heap_t));
    if (hh == NULL) {

        tan_stderr_error(errno, "malloc() failed");
        return NULL;
    }

    hh->arr = (tan_heap_node_t **)calloc(TAN_HEAP_INIT_SIZE,
                                         sizeof(tan_heap_node_t *));

    if (hh->arr == NULL) {
        free(hh);

        tan_stderr_error(errno, "calloc() failed");
        return NULL;
    }

    hh->size = TAN_HEAP_INIT_SIZE;
    hh->used = 0;

    return hh;
}


static tan_int_t
tan_heap_create_node(tan_heap_t *hp, time_t key, void *value)
{
    int                k;
    tan_heap_node_t  **hh_new;

    if (hp->used >= hp->size) {

        hh_new = (tan_heap_node_t **)calloc(2 * hp->size,
                                            sizeof(tan_heap_node_t *));

        if (hh_new == NULL) {

            tan_log_crit(errno, "calloc() failed", NULL);
            return TAN_ERROR;
        }

        /* If array is full double its dimension.  */
        for (k = 0; k < hp->size; ++k)
            hh_new[k] = hp->arr[k];

        free(hp->arr);

        hp->arr  = hh_new;
        hp->size = 2 * hp->size;
    }

    hp->arr[hp->used] = (tan_heap_node_t *)malloc(sizeof(tan_heap_node_t));
    if (hp->arr[hp->used] == NULL) {

        tan_log_crit(errno, "malloc() failed", NULL);
        return TAN_ERROR;
    }

    hp->arr[hp->used]->key   = key;
    hp->arr[hp->used]->value = value;

    ++hp->used;

    return TAN_OK;
}


void *
tan_heap_min_peek(tan_heap_t *hh)
{
    return hh->arr[0]->value;
}


tan_int_t
tan_heap_add_node(tan_heap_t *hh, time_t key, void *value)
{
    int               c, par;
    tan_int_t         ret;
    tan_heap_node_t  *tmp;

    ret = tan_heap_create_node(hh, key, value);
    if (ret != TAN_OK)
        return ret;

    /* upheap  */
    c = hh->used - 1;
    while (c > 0) {

        par = tan_heap_get_parent_index(c);
        if (tan_heap_get_key(hh->arr[c]) >= tan_heap_get_key(hh->arr[par]))
            break;

        /* swap  */
        tmp = hh->arr[c];

        hh->arr[c]   = hh->arr[par];
        hh->arr[par] = tmp;

        c = par;
    }

    return TAN_OK;
}


static int
tan_heap_get_parent_index(int i)
{
    return (i - 2 + (i & 1)) >> 1;
}


static time_t
tan_heap_get_key(tan_heap_node_t *n)
{
    return n->key;
}

static void* 
tan_heap_get_value(tan_heap_node_t *n)
{
    return n->value;
}


void *
tan_heap_remove_min(tan_heap_t *hh)
{
    int               c, l, r;
    void             *out;
    tan_heap_node_t  *tmp;

    if (!hh->used)
        return NULL;

    out = tan_heap_min_peek(hh);
    free(hh->arr[0]);

    /* Swap rightmost leaf and root.  */
    hh->arr[0]            = hh->arr[hh->used - 1];
    hh->arr[hh->used - 1] = NULL;

    --hh->used;

    tan_downheap(hh, 0);

    return out;
}

static int
tan_findNodeByValue(tan_heap_t *hh,
                    int index,
                    void *value)

{
    if(index >= hh->used)
        return -1;

    if(tan_heap_get_value(hh->arr[index]) == value)
        return index;

    int ret;

    ret = tan_findNodeByValue(hh,
                              tan_heap_get_left_index(index),
                              value);

    if(ret == -1)
        ret = tan_findNodeByValue(hh,
                                  tan_heap_get_right_index(index),
                                  value);

    return ret;
}

time_t 
tan_remove_node(tan_heap_t *hh,
                void* value)
{
    int    targetIndex;
    time_t out;

    targetIndex = tan_findNodeByValue(hh, 0, value);

    if(targetIndex == -1){

        return -1;
    }

    out = tan_heap_get_key(hh->arr[targetIndex]);
    free(hh->arr[targetIndex]);

    //swap rightmost leaf and target
    hh->arr[targetIndex] = hh->arr[hh->used-1];
    hh->arr[hh->used-1]  = NULL;
    --hh->used;

    tan_downheap(hh, targetIndex);

    return out;
}

static void 
tan_downheap(tan_heap_t *hh,
             int index)
{
    int              l, r;
    tan_heap_node_t  *tmp;

    while (index < hh->used) 
    {

        l = tan_heap_get_left_index(index);
        r = tan_heap_get_right_index(index);

        if (r < hh->used && l < hh->used &&
            hh->arr[l]->key < hh->arr[r]->key)
        {
            /* Left is smaller.  */
            if (hh->arr[index]->key > hh->arr[l]->key) 
            {
                /* Swap arr[index] and left child.  */
                tmp = hh->arr[index];

                hh->arr[index] = hh->arr[l];
                hh->arr[l] = tmp;

                index = l;
            }
            else 
            {
                break;
            }
        } 
        else
        {
            /* Right is smaller.  */
            if (r < hh->used && hh->arr[index]->key > hh->arr[r]->key)
            {
                /* Swap arr[index] and right child.  */
                tmp = hh->arr[index];

                hh->arr[index] = hh->arr[r];
                hh->arr[r] = tmp;

                index = r;
            } 
            else
            {
                break;
            }
        }
    }
}

static int
tan_heap_get_left_index(int i)
{
    return (i<<1) + 1;
}


static int
tan_heap_get_right_index(int i)
{
    return (i<<1) + 2;
}


void
tan_heap_destory(tan_heap_t *hh)
{
    int  k;

    for (k = 0; k < hh->used; ++k)
        free(hh->arr[k]);

    free(hh->arr);
    free(hh);
}


#if 0
void
tan_heap_print(tan_heap_t *hh)
{
    int  i;

    printf("[");

    for (i = 0; i < hh->used; ++i)
        printf("%lu ", hh->arr[i]->key);

    printf("]\n");
}
#endif

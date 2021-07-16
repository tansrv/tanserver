/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_heap.h"


#define TAN_HEAP_INIT_SIZE  20


static tan_int_t tan_heap_create_node(tan_heap_t *hp, time_t key,
                                      void *value);
static int tan_heap_get_parent_index(int i);
static time_t tan_heap_get_key(tan_heap_node_t *n);
static int tan_heap_get_left_index(int i);
static int tan_heap_get_right_index(int i);


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

    /* downheap  */
    c = 0;
    while (c < hh->used) {

        l = tan_heap_get_left_index(c);
        r = tan_heap_get_right_index(c);

        if (r < hh->used && l < hh->used &&
            hh->arr[l]->key < hh->arr[r]->key)
        {
            /* Left is smaller.  */
            if (hh->arr[c]->key > hh->arr[l]->key) {
                /* Swap arr[c] and left child.  */
                tmp = hh->arr[c];

                hh->arr[c] = hh->arr[l];
                hh->arr[l] = tmp;

                c = l;
            } else {
                break;
            }
        } else {
            /* Right is smaller.  */
            if (r < hh->used && hh->arr[c]->key > hh->arr[r]->key) {
                /* Swap arr[c] and right child.  */
                tmp = hh->arr[c];

                hh->arr[c] = hh->arr[r];
                hh->arr[r] = tmp;

                c = r;
            } else {
                break;
            }
        }
    }    
}

static int
tan_heap_get_left_index(int i)
{
    return (2 * i) + 1;
}


static int
tan_heap_get_right_index(int i)
{
    return (2 * i) + 2;
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

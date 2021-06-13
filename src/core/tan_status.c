/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


#define TAN_SHM_NAME  "tanserver"
#define TAN_SHM_FILE  "/dev/shm/tanserver"


typedef struct {
    time_t                       start_time;
    volatile unsigned            active_connections;
    volatile unsigned long long  handled_requests;
} tan_status_t;


static tan_shm_t  *status;


tan_int_t
tan_status_shm_init()
{
    status = tan_shm_create(TAN_SHM_NAME, tan_get_page_size(),
                            PROT_READ | PROT_WRITE, MAP_SHARED);

    if (status == NULL)
        return TAN_ERROR;

    ((tan_status_t *)status)->start_time         = time(NULL);
    ((tan_status_t *)status)->active_connections = 0;
    ((tan_status_t *)status)->handled_requests   = 0;

    return TAN_OK;
}


tan_int_t
tan_status_shm_attach(int prot, int flags)
{
    status = tan_shm_attach(TAN_SHM_NAME, tan_get_page_size(),
                            prot, flags);

    if (status == NULL)
        return TAN_ERROR;

    return TAN_OK;
}


int
tan_is_running()
{
    return tan_check_file_exists(TAN_SHM_FILE);
}


time_t
tan_get_uptime()
{
    return time(NULL) - ((tan_status_t *)status)->start_time;
}


volatile unsigned
tan_get_active_connections()
{
    return ((tan_status_t *)status)->active_connections;
}


volatile unsigned long long
tan_get_handled_requests()
{
    return ((tan_status_t *)status)->handled_requests;
}


void
tan_active_connections_atomic_inc()
{
    if (tan_unlikely(tan_is_stop()))
        return;

    tan_atomic_inc(((tan_status_t *)status)->active_connections);
}


void
tan_active_connections_atomic_dec()
{
    if (tan_unlikely(tan_is_stop()))
        return;

    tan_atomic_dec(((tan_status_t *)status)->active_connections);
}


void
tan_handled_requests_atomic_inc()
{
    if (tan_unlikely(tan_is_stop()))
        return;

    tan_atomic_inc(((tan_status_t *)status)->handled_requests);
}


void
tan_status_shm_munmap()
{
    tan_munmap(status, tan_get_page_size());
}


void
tan_status_shm_free()
{
    tan_shm_free(status, tan_get_page_size(), TAN_SHM_NAME);
}

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


static struct {
    int  core_count;
    int  page_size;
    int  open_files_limit;
} syscfg;


tan_int_t
tan_syscfg_init()
{
    struct rlimit  limit;

    syscfg.core_count = sysconf(_SC_NPROCESSORS_ONLN);
    if (syscfg.core_count == -1) {

        tan_stderr_error(errno, "sysconf(_SC_NPROCESSORS_ONLN) failed");
        return TAN_ERROR;
    }

    syscfg.page_size = sysconf(_SC_PAGESIZE);
    if (syscfg.page_size == -1) {

        tan_stderr_error(errno, "sysconf(_SC_PAGESIZE) failed");
        return TAN_ERROR;
    }

    if (getrlimit(RLIMIT_NOFILE, &limit)) {

        tan_stderr_error(errno, "getrlimit() failed");
        return TAN_ERROR;
    }

    syscfg.open_files_limit = limit.rlim_cur;
    return TAN_OK;
}


int
tan_get_core_count()
{
    return syscfg.core_count;
}


int
tan_get_page_size()
{
    return syscfg.page_size;
}


int
tan_get_open_files_limit()
{
    return syscfg.open_files_limit;
}

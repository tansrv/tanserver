/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


void
tan_setaffinity(int core)
{
    cpu_set_t  set;

    CPU_ZERO(&set);
    CPU_SET(core, &set);

    if (sched_setaffinity(0, sizeof(set), &set))
        tan_stderr_error(errno, "sched_setaffinity() failed");
}

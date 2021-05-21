/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


static void tan_signal_handler(int signo, siginfo_t *info, void *ctx);


static int  linux_signals[] = {
    SIGUSR1, SIGUSR2,
    SIGPIPE, SIGABRT,
    0,
};

static int  stop;
static int  reload;


tan_int_t
tan_signal_init()
{
    int               k;
    struct sigaction  sa;

    sa.sa_flags     = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = tan_signal_handler;

    sigemptyset(&sa.sa_mask);

    for (k = 0; linux_signals[k]; ++k) {

        if (sigaction(linux_signals[k], &sa, NULL)) {

            tan_stderr_error(errno, "sigaction() failed");
            return TAN_ERROR;
        }
    }

    return TAN_OK;
}


static void
tan_signal_handler(int signo, siginfo_t *info, void *ctx)
{
    switch (signo) {

    case SIGUSR1:

        stop = 1;
        break;

    case SIGUSR2:

        reload = 1;
        break;
    }
}


int
tan_is_stop()
{
    return stop;
}


int
tan_start_reload()
{
    return reload;
}


void
tan_reload_end()
{
    reload = 0;
}

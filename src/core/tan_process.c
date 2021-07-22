/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_process.h"


#define TAN_PIDFILE   "/var/run/tanserver.pid"
#define TAN_DEV_NULL  "/dev/null"


tan_int_t
tan_daemon()
{
    tan_fd_t  fd;

    switch (fork()) {

    case 0:

        break;

    case -1:

        tan_stderr_error(errno, "fork() failed");
        return TAN_ERROR;

    default:

        exit(0);
    }

    if (setsid() == -1) {

        tan_stderr_error(errno, "setsid() failed");
        return TAN_ERROR;
    }

    umask(0);

    fd = open(TAN_DEV_NULL, O_RDWR);
    if (fd == -1) {

        tan_stderr_error(errno, "open(\"%s\") failed",
                         TAN_DEV_NULL);

        return TAN_ERROR;
    }

    if (dup2(fd, STDIN_FILENO) == -1 ||
        dup2(fd, STDOUT_FILENO) == -1)
    {
        tan_stderr_error(errno, "dup2() failed");
        return TAN_ERROR;
    }

    tan_close_file(fd);
    return TAN_OK;
}


tan_int_t
tan_create_pidfile()
{
    char      buf[20];
    tan_fd_t  fd;

    fd = open(TAN_PIDFILE, O_CREAT | O_WRONLY, 0644);
    if (fd == -1) {

        tan_stderr_error(errno, "open(\"%s\") failed",
                         TAN_PIDFILE);

        return TAN_ERROR;
    }

    tan_memzero(buf, 20);
    snprintf(buf, 20, "%d", getpid());

    if (write(fd, buf, strlen(buf)) == -1) {

        tan_stderr_error(errno, "write(\"%s\") failed",
                         TAN_PIDFILE);

        tan_close_file(fd);
        return TAN_ERROR;
    }

    tan_close_file(fd);
    return TAN_OK;
}


void
tan_remove_pidfile()
{
    if (remove(TAN_PIDFILE)) {
        tan_log_warn(errno, "remove(\"%s\") failed",
                     TAN_PIDFILE);
    }
}


void
tan_set_process_name(const char *name)
{
    if (prctl(PR_SET_NAME, name)) {
        tan_stderr_error(errno, "prctl(PR_SET_NAME, "
                         "\"%s\") failed", name);
    }
}

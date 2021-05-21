/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


#define TAN_ERROR_LOG   "error.log"
#define TAN_ACCESS_LOG  "access.log"


static tan_int_t tan_open_error_log();
static tan_int_t tan_open_access_log();
static tan_fd_t tan_open_log_file(const char *path);
static void tan_insert_errno(char *buf, int err, int *k);
static void tan_insert_lf(char *buf, int *k);
static void tan_write_string_to_stderr(const char *buf, int k);


static const char  *log_levels[] = {
    "  CRIT  ",
    "  WARN  ",
    "  INFO  ",
    "DATABASE",
};


static struct {
    tan_fd_t  error;
    tan_fd_t  access;
} log_fds;


tan_int_t
tan_log_init()
{
    tan_int_t  ret;

    ret = tan_open_error_log();
    if (ret != TAN_OK)
        return ret;

    return tan_open_access_log();
}


static tan_int_t
tan_open_error_log()
{
    char  path[TAN_PATH_SIZE + 50];

    tan_memzero(path, TAN_PATH_SIZE + 50);

    snprintf(path, TAN_PATH_SIZE + 50, "%s"TAN_ERROR_LOG,
             tan_get_log_cfg()->directory);

    log_fds.error = tan_open_log_file(path);
    if (log_fds.error == -1)
        return TAN_ERROR;

    return TAN_OK;
}


static tan_int_t
tan_open_access_log()
{
    char  path[TAN_PATH_SIZE + 50];

    tan_memzero(path, TAN_PATH_SIZE + 50);

    snprintf(path, TAN_PATH_SIZE + 50, "%s"TAN_ACCESS_LOG,
             tan_get_log_cfg()->directory);

    log_fds.access = tan_open_log_file(path);
    if (log_fds.access == -1)
        return TAN_ERROR;

    return TAN_OK;
}


static tan_fd_t
tan_open_log_file(const char *path)
{
    tan_fd_t  fd;

    fd = open(path, O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd == -1)
        tan_stderr_error(errno, "open(\"%s\") failed", path);

    return fd;
}


tan_fd_t
tan_get_access_log()
{
    return log_fds.access;
}


void
tan_log(int err, int level, int insert_lf,
        const char *func, const char *str, ...)
{
    int      k;
    char     buf[TAN_MAX_STR_SIZE];
    va_list  arg;

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    k = snprintf(buf, TAN_MAX_STR_SIZE,
                 "[%s] [%s] [%d] ", log_levels[level],
                 tan_get_localtime_str(), getpid());

    if (func != NULL)
        k += snprintf(buf + k, TAN_MAX_STR_SIZE - k, "%s(): ", func);

    va_start(arg, str);
    vsnprintf(buf + k, TAN_MAX_STR_SIZE - k, str, arg);
    va_end(arg);

    k = strlen(buf);

    if (err)
        tan_insert_errno(buf, err, &k);

    if (insert_lf)
        tan_insert_lf(buf, &k);

    if (write(log_fds.error, buf, k) == -1)
        tan_stderr_info(buf);
}


void
tan_stderr_error(int err, const char *str, ...)
{
    int      k;
    char     buf[TAN_MAX_STR_SIZE];
    va_list  arg;

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    memcpy(buf, "tanserver: ", 11);

    k = 11;

    va_start(arg, str);
    vsnprintf(buf + k, TAN_MAX_STR_SIZE - k, str, arg);
    va_end(arg);

    k = strlen(buf);

    if (err)
        tan_insert_errno(buf, err, &k);

    tan_insert_lf(buf, &k);

    tan_write_string_to_stderr(buf, k);
}


void
tan_stderr_info(const char *str, ...)
{
    int      k;
    char     buf[TAN_MAX_STR_SIZE];
    va_list  arg;

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    va_start(arg, str);
    vsnprintf(buf, TAN_MAX_STR_SIZE, str, arg);
    va_end(arg);

    k = strlen(buf);

    tan_insert_lf(buf, &k);

    tan_write_string_to_stderr(buf, k);
}


static void
tan_insert_errno(char *buf, int err, int *k)
{
    snprintf(buf + *k, TAN_MAX_STR_SIZE - *k,
             " (%d: %s)", err, strerror(err));

    *k = strlen(buf);
}


static void
tan_insert_lf(char *buf, int *k)
{
    if (TAN_MAX_STR_SIZE - *k > 1) {
        *(buf + *k) = '\n';
        ++*k;
    } else {
        *(buf + *k - 1) = '\n';
    }
}


static void
tan_write_string_to_stderr(const char *buf, int k)
{
    fwrite(buf, 1, k, stderr);
    fflush(stderr);
}

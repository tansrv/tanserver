/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_LOG_H
#define TAN_LOG_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


typedef enum {
    TAN_LOG_CRIT = 0,
    TAN_LOG_WARN,
    TAN_LOG_INFO,
    TAN_LOG_DATABASE,
} tan_log_levels_e;


tan_int_t tan_log_init();
tan_fd_t tan_get_access_log();
void tan_log(int err, int level, int insert_lf,
             const char *func, const char *str, ...);
void tan_stderr_error(int err, const char *str, ...);
/* without "tanserver:"  */
void tan_stderr_info(const char *str, ...);


#define tan_log_crit(err, str, ...)          \
    tan_log(err, TAN_LOG_CRIT, 1,            \
            __func__, str, __VA_ARGS__)

#define tan_log_warn(err, str, ...)          \
    tan_log(err, TAN_LOG_WARN, 1,            \
            __func__, str, __VA_ARGS__)

#define tan_log_info(str, ...)               \
    tan_log(0, TAN_LOG_INFO, 1,              \
            NULL, str, __VA_ARGS__)

#define tan_log_database(func, str, ...)     \
    tan_log(0, TAN_LOG_DATABASE, 0,          \
            func, str, __VA_ARGS__)

#define tan_log_database_lf(func, str, ...)  \
    tan_log(0, TAN_LOG_DATABASE, 1,          \
            func, str, __VA_ARGS__)


#ifdef __cplusplus
}
#endif


#endif /* TAN_LOG_H */

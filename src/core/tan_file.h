/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_FILE_H
#define TAN_FILE_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


typedef int  tan_fd_t;


off_t tan_get_filesize(const char *path);
int tan_check_file_exists(const char *path);
FILE *tan_open_file(const char *path, const char *modes,
                    long long offset, int whence);
tan_int_t tan_fread(FILE *fp, void *buf, size_t n);
tan_int_t tan_file_copy(const char *src, const char *dest);


#define tan_close_file(fd)  close(fd)


#ifdef __cplusplus
}
#endif


#endif /* TAN_FILE_H */

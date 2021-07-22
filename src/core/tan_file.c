/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_file.h"


off_t
tan_get_filesize(const char *path)
{
    struct stat  filestat;

    if (stat(path, &filestat)) {

        tan_log_warn(errno, "stat(\"%s\") failed", path);
        return -1;
    }

    return filestat.st_size;
}


int
tan_check_file_exists(const char *path)
{
    if (access(path, F_OK))
        return -1;

    return 0;
}


FILE *
tan_open_file(const char *path, const char *modes,
              long long offset, int whence)
{
    FILE  *fp;

    fp = fopen(path, modes);
    if (fp == NULL) {

        tan_log_warn(errno, "fopen(\"%s\") failed", path);
        return NULL;
    }

    if (fseeko(fp, offset, whence)) {
        tan_log_warn(errno, "fseeko(\"%s\") failed", path);

        fclose(fp);
        return NULL;
    }

    return fp;
}


tan_int_t
tan_fread(FILE *fp, void *buf, size_t n)
{
    if (fread(buf, 1, n, fp) == n)
        return TAN_OK;

    return TAN_ERROR;
}


tan_int_t
tan_file_copy(const char *src, const char *dest)
{
    FILE   *from, *to;
    char    buf[TAN_MAX_STR_SIZE];
    size_t  len;

    from = fopen(src, "r");
    if (from == NULL) {

        tan_log_warn(errno, "fopen(\"%s\") failed", src);
        return TAN_ERROR;
    }

    to = fopen(dest, "w");
    if (to == NULL) {
        tan_log_warn(errno, "fopen(\"%s\") failed", dest);

        fclose(from);
        return TAN_ERROR;
    }

    while (len = fread(buf, 1, TAN_MAX_STR_SIZE - 1, from))
        fwrite(buf, 1, len, to);

    fclose(from);
    fclose(to);

    return TAN_OK;
}

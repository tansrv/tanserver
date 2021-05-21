/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"


tan_shm_t *
tan_shm_create(const char *name, size_t size,
               int prot, int flags)
{
    tan_fd_t    fd;
    tan_shm_t  *shm;

    fd = shm_open(name, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {

        tan_stderr_error(errno, "shm_open(\"%s\") failed", name);
        return NULL;
    }

    if (ftruncate(fd, size)) {

        tan_stderr_error(errno, "ftruncate() failed");
        goto failed;
    }

    shm = mmap(NULL, size, prot, flags, fd, 0);
    if (shm == MAP_FAILED) {

        tan_stderr_error(errno, "mmap() failed");
        goto failed;
    }

    tan_close_file(fd);
    return shm;

failed:

    shm_unlink(name);
    tan_close_file(fd);

    return NULL;
}


tan_shm_t *
tan_shm_attach(const char *name, size_t size,
               int prot, int flags)
{
    return tan_shm_create(name, size, prot, flags);
}


void
tan_munmap(tan_shm_t *shm, size_t size)
{
    munmap(shm, size);
}


void
tan_shm_free(tan_shm_t *shm, size_t length, const char *name)
{
    munmap(shm, length);
    shm_unlink(name);
}

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_LINUX_CONF_H
#define TAN_LINUX_CONF_H


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define _POSIX_C_SOURCE  200112L


#include <dirent.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <string.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <netinet/tcp.h>
#include <sys/sysinfo.h>


#endif /* TAN_LINUX_CONF_H */

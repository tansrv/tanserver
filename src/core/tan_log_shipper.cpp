/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_openssl_ssl.h"


#define TAN_FILENAME_SIZE  50


typedef struct {
    char       hostname[TAN_HOSTNAME_SIZE];
    int        file_count;
} __attribute__ ((packed)) tan_hostinfo_t;


typedef struct {
    char       filename[TAN_FILENAME_SIZE];
    long long  offset;
} __attribute__ ((packed)) tan_fileinfo_t;


static void tan_hostname_init(tan_hostinfo_t *hi);
static DIR *tan_open_log_directory();
static void tan_client_connected(tan_socket_t fd, DIR *dir,
                                 tan_hostinfo_t *hi);

static tan_int_t tan_read_allowlist(tan_socket_t fd,
                                    struct sockaddr_in *addr);
static void tan_start_shipping(SSL *ssl, tan_hostinfo_t *hi, DIR *dir);

static tan_int_t tan_send_hostinfo(SSL *ssl, DIR *dir, tan_hostinfo_t *hi);
static int tan_get_localhost_filecount(DIR *dir);

static tan_int_t tan_send_fileinfo(SSL *ssl, DIR *dir);
static tan_int_t tan_recv_filecount(SSL *ssl, int *file_count);

static void tan_recv_fileinfo(SSL *ssl, int file_count);
static tan_int_t tan_send_filesize(SSL *ssl, long long size);
static tan_int_t tan_send_file(SSL *ssl, FILE *fp, long long size);

static tan_int_t tan_ssl_handshake1(SSL *ssl);
static tan_int_t tan_ssl_read1(SSL *ssl, void *buf, int num);
static tan_int_t tan_ssl_write1(SSL *ssl, void *buf, int num);


/*
 * 1. Server: Send the number of log files and hostname to the client.
 * 2. Client: Create a directory (hostname).
 * 3. Server: Send the information (file name + size) of
 *            each log file to the client.
 * 4. Client: Receive log file information one by one and push it
 *            into a container according to the file name and size.
 * 5. Client: Send the number of files that should be received
 *            and the information (file name + size) of each file to the server.
 * 6. Server: Receive the information of each file one by one
 *            according to the number of files, send the size that the
 *            client should receive, open the file and send it.
 */
void
tan_shipper_process_loop()
{
    fd_set          fds;
    DIR            *dir;
    tan_socket_t    shipper;
    tan_hostinfo_t  hi;

    tan_hostname_init(&hi);

    dir = tan_open_log_directory();
    if (dir == NULL)
        exit(-1);

    if (tan_create_listen_socket(&shipper,
        tan_get_shipper_cfg()->port) != TAN_OK)
    {
        exit(-1);
    }

    FD_ZERO(&fds);
    FD_SET(shipper, &fds);

    while (!tan_is_stop()) {

        select(shipper + 1, &fds, (fd_set *)0,
               (fd_set *)0, (struct timeval *)0);

        tan_time_update();

        tan_client_connected(shipper, dir, &hi);
    }

    closedir(dir);
    tan_close_socket(shipper);

    exit(0);
}


static void
tan_hostname_init(tan_hostinfo_t *hi)
{
    tan_memzero(hi->hostname, TAN_HOSTNAME_SIZE);
    snprintf(hi->hostname, TAN_HOSTNAME_SIZE,
             "%s", tan_get_shipper_cfg()->hostname);
}


static DIR *
tan_open_log_directory()
{
    DIR  *dir;

    dir = opendir(tan_get_log_cfg()->directory);
    if (dir == NULL) {
        tan_stderr_error(errno, "opendir(\"%s\") failed",
                         tan_get_log_cfg()->directory);
    }

    return dir;
}


static void
tan_client_connected(tan_socket_t fd, DIR *dir,
                     tan_hostinfo_t *hi)
{
    SSL                *ssl;
    tan_socket_t        client;
    socklen_t           addrlen;
    struct sockaddr_in  addr;

    addrlen = sizeof(struct sockaddr_in);

    for (;;) {

        client = accept4(fd, (struct sockaddr *)&addr,
                         &addrlen, SOCK_NONBLOCK);

        if (client == -1) {

            if (errno == EAGAIN)
                break;

            if (errno == EINTR || errno == ECONNABORTED)
                continue;

            tan_log_warn(errno, "accept4() failed", NULL);
            break;
        }

        ssl = NULL;

        if (tan_read_allowlist(client, &addr) != TAN_OK)
            goto failed;

        ssl = tan_ssl_new(client);
        if (ssl == NULL)
            goto failed;

        if (tan_ssl_handshake1(ssl) != TAN_OK)
            goto failed;

        tan_start_shipping(ssl, hi, dir);

        SSL_shutdown(ssl);

    failed:

        tan_ssl_close(&ssl, &client);
    }
}


static tan_int_t
tan_read_allowlist(tan_socket_t fd,
                   struct sockaddr_in *addr)
{
    char           hostaddr[TAN_HOSTADDR_SIZE];
    int            k;
    const u_char  *p;

    p = tan_get_hostaddr(addr);

    tan_memzero(hostaddr, TAN_HOSTADDR_SIZE);

    snprintf(hostaddr, TAN_HOSTADDR_SIZE,
             "%d.%d.%d.%d",
             p[0], p[1], p[2], p[3]);

    for (k = 0; k < tan_get_shipper_cfg()->allowlist.size; ++k) {

        if (!strcmp(hostaddr,
            tan_get_shipper_cfg()->allowlist.vec[k])) {

            send(fd, "0", 1, 0);
            return TAN_OK;
        }
    }

    send(fd, "1", 1, 0);
    return TAN_ERROR;
}


static void
tan_start_shipping(SSL *ssl, tan_hostinfo_t *hi, DIR *dir)
{
    int  file_count;

    if (tan_send_hostinfo(ssl, dir, hi) != TAN_OK)
        return;

    if (tan_send_fileinfo(ssl, dir) != TAN_OK)
        return;

    if (tan_recv_filecount(ssl, &file_count) != TAN_OK)
        return;

    tan_recv_fileinfo(ssl, file_count);
}


static tan_int_t
tan_send_hostinfo(SSL *ssl, DIR *dir, tan_hostinfo_t *hi)
{
    hi->file_count = htonl(tan_get_localhost_filecount(dir));

    return tan_ssl_write1(ssl, (char *)hi,
                          sizeof(tan_hostinfo_t));
}


static int
tan_get_localhost_filecount(DIR *dir)
{
    int             count;
    struct dirent  *de;

    count = 0;
    rewinddir(dir);

    while (de = readdir(dir)) {

        if (strstr(de->d_name, ".log") != NULL)
            ++count;
    }

    return count;
}


static tan_int_t
tan_send_fileinfo(SSL *ssl, DIR *dir)
{
    char            path[TAN_PATH_SIZE + TAN_FILENAME_SIZE];
    struct dirent  *de;
    tan_int_t       ret;
    tan_fileinfo_t  fi;

    rewinddir(dir);

    while (de = readdir(dir)) {

        if (strstr(de->d_name, ".log") != NULL) {

            tan_memzero(&fi, sizeof(tan_fileinfo_t));

            strcpy(fi.filename, de->d_name);

            tan_memzero(path, TAN_PATH_SIZE + TAN_FILENAME_SIZE);

            snprintf(path, TAN_PATH_SIZE + TAN_FILENAME_SIZE,
                     "%s%s", tan_get_log_cfg()->directory,
                     fi.filename);

            fi.offset = tan_get_filesize(path);
            if (fi.offset == -1)
                return TAN_ERROR;

            fi.offset = htonl(fi.offset);

            ret = tan_ssl_write1(ssl, (char *)&fi,
                                 sizeof(tan_fileinfo_t));

            if (ret != TAN_OK)
                return ret;
        }
    }

    return TAN_OK;
}


static tan_int_t
tan_recv_filecount(SSL *ssl, int *file_count)
{
    tan_int_t  ret;

    ret = tan_ssl_read1(ssl, (char *)file_count, sizeof(int));
    if (ret != TAN_OK)
        return ret;

    *file_count = ntohl(*file_count);

    if (!*file_count)
        return TAN_ERROR;

    return TAN_OK;
}


static void
tan_recv_fileinfo(SSL *ssl, int file_count)
{
    int             k;
    char            path[TAN_PATH_SIZE + TAN_FILENAME_SIZE];
    FILE           *fp;
    long long       size;
    tan_fileinfo_t  fi;

    for (k = 0; k < file_count; ++k) {

        if (tan_ssl_read1(ssl, (char *)&fi,
                          sizeof(tan_fileinfo_t))
            != TAN_OK)
        {
            return;
        }

        fi.offset = ntohl(fi.offset);

        tan_memzero(path, TAN_PATH_SIZE + TAN_FILENAME_SIZE);

        snprintf(path, TAN_PATH_SIZE + TAN_FILENAME_SIZE,
                 "%s%s", tan_get_log_cfg()->directory,
                 fi.filename);

        size = tan_get_filesize(path);
        if (size == -1)
            return;

        size -= fi.offset;

        if (tan_send_filesize(ssl, htonl(size)) != TAN_OK)
            return;

        fp = tan_open_file(path, "r", fi.offset, SEEK_SET);
        if (fp == NULL)
            return;

        if (tan_send_file(ssl, fp, size) != TAN_OK) {

            fclose(fp);
            return;
        }

        fclose(fp);
    }
}


static tan_int_t
tan_send_filesize(SSL *ssl, long long size)
{
    return tan_ssl_write1(ssl, (char *)&size, sizeof(long long));
}


static tan_int_t
tan_send_file(SSL *ssl, FILE *fp, long long size)
{
    char       buf[SSL3_RT_MAX_PLAIN_LENGTH + 1];
    tan_int_t  ret;

    tan_memzero(buf, SSL3_RT_MAX_PLAIN_LENGTH + 1);

    while (size > SSL3_RT_MAX_PLAIN_LENGTH) {

        ret = tan_fread(fp, buf, SSL3_RT_MAX_PLAIN_LENGTH);
        if (ret != TAN_OK)
            return ret;

        ret = tan_ssl_write1(ssl, buf,
                             SSL3_RT_MAX_PLAIN_LENGTH);

        if (ret != TAN_OK)
            return ret;

        size -= SSL3_RT_MAX_PLAIN_LENGTH;
    }

    ret = tan_fread(fp, buf, size);
    if (ret != TAN_OK)
        return ret;

    return tan_ssl_write1(ssl, buf, size);
}


static tan_int_t
tan_ssl_handshake1(SSL *ssl)
{
    int  failed;

    for (failed = 0; failed < 50; ++failed) {

        if (tan_ssl_accept(ssl) == TAN_SSL_ACCEPT_OK)
            return TAN_OK;

        tan_msleep(100);
    }

    return TAN_ERROR;
}


static tan_int_t
tan_ssl_read1(SSL *ssl, void *buf, int num)
{
    int  failed;

    for (failed = 0; failed < 50; ++failed) {

        if (SSL_read(ssl, buf, num) > 0)
            return TAN_OK;

        tan_msleep(100);
    }

    return TAN_ERROR;
}


static tan_int_t
tan_ssl_write1(SSL *ssl, void *buf, int num)
{
    int  failed;

    for (failed = 0; failed < 50; ++failed) {

        if (SSL_write(ssl, buf, num) > 0)
            return TAN_OK;

        tan_msleep(100);
    }

    return TAN_ERROR;
}

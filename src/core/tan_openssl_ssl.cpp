/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_openssl_ssl.h"


static void tan_write_openssl_error_to_stderr();
static tan_int_t tan_ssl_ctx_init(const char *cert, const char *key);
static tan_ssl_t tan_ssl_get_error(SSL *ssl, int ret);


static SSL_CTX  *ssl_ctx;


tan_int_t
tan_ssl_init()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    ssl_ctx = SSL_CTX_new(SSLv23_method());
    if (ssl_ctx == NULL) {
        tan_write_openssl_error_to_stderr();

        tan_stderr_error(0, "SSL_CTX_new() failed");
        return TAN_ERROR;
    }

    return tan_ssl_ctx_init(tan_get_ssl_cfg()->ssl_cert_file,
                            tan_get_ssl_cfg()->ssl_key_file);
}


static tan_int_t
tan_ssl_ctx_init(const char *cert, const char *key)
{
    if (SSL_CTX_use_certificate_chain_file(ssl_ctx, cert)
        != 1)
    {
        tan_write_openssl_error_to_stderr();

        tan_stderr_error(0, "SSL_CTX_use_certificate_chain_file() failed");
        return TAN_ERROR;
    }

    if (SSL_CTX_use_PrivateKey_file(ssl_ctx, key,
                                    SSL_FILETYPE_PEM)
        != 1)
    {
        tan_write_openssl_error_to_stderr();

        tan_stderr_error(0, "SSL_CTX_use_PrivateKey_file() failed");
        return TAN_ERROR;
    }

    if (SSL_CTX_check_private_key(ssl_ctx)
        != 1)
    {
        tan_write_openssl_error_to_stderr();

        tan_stderr_error(0, "SSL_CTX_check_private_key() failed");
        return TAN_ERROR;
    }

    SSL_CTX_set_quiet_shutdown(ssl_ctx, 1);
    return TAN_OK;
}


static void
tan_write_openssl_error_to_stderr()
{
    ERR_print_errors_fp(stderr);
}


SSL *
tan_ssl_new(tan_socket_t fd)
{
    SSL  *ssl;

    ssl = SSL_new(ssl_ctx);
    if (ssl == NULL) {

        tan_log_crit(0, "SSL_new() failed", NULL);
        return NULL;
    }

    if (SSL_set_fd(ssl, fd) != 1) {
        SSL_free(ssl);

        tan_log_crit(0, "SSL_set_fd() failed", NULL);
        return NULL;
    }

    return ssl;
}


tan_ssl_t
tan_ssl_accept(SSL *ssl)
{
    int  ret;

    ret = SSL_accept(ssl);
    if (ret == 1)
        return TAN_SSL_ACCEPT_OK;

    return tan_ssl_get_error(ssl, ret);
}


tan_ssl_t
tan_ssl_read(int *bytes_read, SSL *ssl, void *buf, int num)
{
    int  bytes, ret;

    for (;;) {

        bytes = *bytes_read;

        *bytes_read += SSL_read(ssl, (char *)buf + *bytes_read,
                                num - *bytes_read);

        if (*bytes_read == num)
            return TAN_SSL_READ_OK;

        ret = *bytes_read - bytes;

        if (ret > 0)
            continue;
        else if (ret < 0)
            ++*bytes_read; /* may return TAN_SSL_CONTINUE  */

        return tan_ssl_get_error(ssl, ret);
    }
}


tan_ssl_t
tan_ssl_readline(SSL *ssl, std::string &line)
{
    char       buf[2];
    int        len, bytes_read;
    tan_ssl_t  ret;

    buf[1]     = '\0';
    len        = line.length();
    bytes_read = 0;

    for (;;) {

        ret = tan_ssl_read(&bytes_read, ssl, buf, 1);

        switch (ret) {

        case TAN_SSL_READ_OK:

            break;

        case TAN_SSL_CONTINUE:

            return TAN_SSL_READLINE_CONTINUE;

        default:

            return TAN_SSL_READLINE_FAILED;
        }

        line.append(buf);

        if (++len > TAN_MAX_STR_SIZE)
            return TAN_SSL_READLINE_TOO_LARGE;

        if (buf[0] == '\n')
            return TAN_SSL_READLINE_DONE;
    }
}


tan_ssl_t
tan_ssl_write(SSL *ssl, const void *buf, int num)
{
    int  ret;

    ret = SSL_write(ssl, buf, num);
    if (ret > 0)
        return TAN_SSL_WRITE_OK;

    return tan_ssl_get_error(ssl, ret);
}


static tan_ssl_t
tan_ssl_get_error(SSL *ssl, int ret)
{
    switch (SSL_get_error(ssl, ret)) {

    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE:

        return TAN_SSL_CONTINUE;

    case SSL_ERROR_ZERO_RETURN:

        return TAN_SSL_DISCONNECTED;

    default:

        return TAN_SSL_OTHER_ERROR;
    }
}


void
tan_ssl_close(SSL **ssl, tan_socket_t *fd)
{
    if (*ssl != NULL) {
        SSL_free(*ssl);
        *ssl = NULL;
    }

    if (*fd != -1) {
        tan_close_socket(*fd);
        *fd = -1;
    }
}

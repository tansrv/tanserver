/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_OPENSSL_SSL_H
#define TAN_OPENSSL_SSL_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"

#include <openssl/ssl.h>
#include <openssl/err.h>


typedef int  tan_ssl_t;


typedef enum {
    TAN_SSL_CONTINUE = 0,
    TAN_SSL_DISCONNECTED,
    TAN_SSL_OTHER_ERROR,
    TAN_SSL_READ_OK,
    TAN_SSL_WRITE_OK,
    TAN_SSL_ACCEPT_OK
} tan_ssl_status_e;


tan_int_t tan_ssl_init();
SSL *tan_ssl_new(tan_socket_t fd);
tan_ssl_t tan_ssl_accept(SSL *ssl);

/*
 * If num > 16384 and return TAN_SSL_CONTINUE,
 * the parameters of the next call should be the same
 */
tan_ssl_t tan_ssl_read(int *bytes_read, SSL *ssl, void *buf, int num);

tan_ssl_t tan_ssl_write(SSL *ssl, const void *buf, int num);
void tan_ssl_close(SSL **ssl, tan_socket_t *fd);


#ifdef __cplusplus
}
#endif


#endif /* TAN_OPENSSL_SSL_H */

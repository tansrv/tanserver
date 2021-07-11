/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_OPENSSL_SSL_H
#define TAN_OPENSSL_SSL_H


#include "tan_core.h"

#include <string>
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


typedef enum {
    TAN_SSL_READLINE_FAILED = 0,
    TAN_SSL_READLINE_CONTINUE,
    TAN_SSL_READLINE_TOO_LARGE,
    TAN_SSL_READLINE_DONE,
} tan_ssl_readline_status_e;


tan_int_t tan_ssl_init();
SSL *tan_ssl_new(tan_socket_t fd);

/**
 * Start handshake.
 *
 * @return TAN_SSL_CONTINUE     ||
 *         TAN_SSL_DISCONNECTED ||
 *         TAN_SSL_OTHER_ERROR  ||
 *         TAN_SSL_ACCEPT_OK
 */
tan_ssl_t tan_ssl_accept(SSL *ssl);

/**
 * Read data of the specified length from SSL socket
 * and save it in the specified buffer.
 *
 * @param bytes_read: The length that has been read
 * @param buf:        The specified buffer
 * @param num:        The specified length
 *
 * @return TAN_SSL_CONTINUE     ||
 *         TAN_SSL_DISCONNECTED ||
 *         TAN_SSL_OTHER_ERROR  ||
 *         TAN_SSL_READ_OK
 *
 * @note If num > 16384 and return TAN_SSL_CONTINUE,
 *       the parameters of the next call should be the same.
 */
tan_ssl_t tan_ssl_read(int *bytes_read, SSL *ssl, void *buf, int num);

/**
 * Read line from SSL socket and save it in the specified buffer.
 *
 * @param line: The specified buffer
 *
 * @return TAN_SSL_READLINE_FAILED    ||
 *         TAN_SSL_READLINE_CONTINUE  ||
 *         TAN_SSL_READLINE_TOO_LARGE ||
 *         TAN_SSL_READLINE_DONE
 *
 * @note If TAN_SSL_READLINE_CONTINUE is returned, should continue
 *       to call this function until TAN_SSL_READLINE_DONE is returned.
 */
tan_ssl_t tan_ssl_readline(SSL *ssl, std::string &line);

/**
 * Send data of the specified length in the specified buffer to the client.
 *
 * @param buf: The specified buffer
 * @param num: The specified length
 *
 * @return TAN_SSL_CONTINUE     ||
 *         TAN_SSL_DISCONNECTED ||
 *         TAN_SSL_OTHER_ERROR  ||
 *         TAN_SSL_WRITE_OK
 *
 * @note If it returns TAN_SSL_CONTINUE, the parameters
 *       of the next call should be the same.
 */
tan_ssl_t tan_ssl_write(SSL *ssl, const void *buf, int num);

void tan_ssl_close(SSL **ssl, tan_socket_t *fd);


#endif /* TAN_OPENSSL_SSL_H */

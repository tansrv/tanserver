/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_OPENSSL_CRYPTO_H
#define TAN_OPENSSL_CRYPTO_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


#define TAN_SHA1_LENGTH  20  /* 160 bits  */


void tan_sha1_encode(const unsigned char *in, size_t n,
                     unsigned char *out);

/**
 * @param buf: Data to be encoded
 * @param len: Length of the data to be encoded
 *
 * @return Base64 string
 *
 * @note Caller is responsible for freeing the returned buffer.
 */
char *tan_base64_encode(const char *buf, int len);


#ifdef __cplusplus
}
#endif


#endif /* TAN_OPENSSL_CRYPTO_H */

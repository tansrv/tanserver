/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_openssl_crypto.h"

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>


void
tan_sha1_encode(const unsigned char *in, size_t n,
                unsigned char *out)
{
    SHA1(in, n, out);
}


char *
tan_base64_encode(const char *buf, int len)
{
    char     *res;
    BIO      *mem, *base64;
    BUF_MEM  *p;

    base64 = BIO_new(BIO_f_base64());
    if (base64 == NULL) {

        tan_log_crit(0, "BIO_new(BIO_f_base64()) failed", NULL);
        return NULL;
    }

    BIO_set_flags(base64, BIO_FLAGS_BASE64_NO_NL);

    res = NULL;

    mem = BIO_new(BIO_s_mem());
    if (mem == NULL) {

        tan_log_crit(0, "BIO_new(BIO_s_mem()) failed", NULL);
        goto out_release;
    }

    BIO_push(base64, mem);
    BIO_write(base64, buf, len);
    BIO_flush(base64);

    BIO_get_mem_ptr(base64, &p);

    res = (char *)calloc(1, p->length + 1);
    if (res == NULL) {

        tan_log_crit(errno, "calloc() failed", NULL);
        goto out_release;
    }

    memcpy(res, p->data, p->length);

out_release:

    BIO_free_all(base64);
    return res;
}

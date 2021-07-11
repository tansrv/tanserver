#ifndef TAN_HTTP_H
#define TAN_HTTP_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


char *tan_http_header_get_value(const char *header,
                                const char *key);


#ifdef __cplusplus
}
#endif


#endif /* TAN_HTTP_H */

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_STRING_H
#define TAN_STRING_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


#define TAN_STRING_BRIGHT_GREEN  "\033[1m\033[32m"
#define TAN_STRING_COLOR_END     "\033[0m"


tan_vector_t(string, char *);


#define tan_memzero(buf, n)  memset(buf, 0, n);


#ifdef __cplusplus
}
#endif


#endif /* TAN_STRING_H */

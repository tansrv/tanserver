/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_STRING_H
#define TAN_STRING_H


#include "tan_core.h"


#define TAN_STRING_BRIGHT_GREEN  "\033[1m\033[32m"
#define TAN_STRING_COLOR_END     "\033[0m"


tan_vector_t(string, char *);


#define tan_memzero(buf, n)  memset(buf, 0, n);


#endif /* TAN_STRING_H */

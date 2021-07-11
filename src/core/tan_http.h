/*
 * Copyright (C) tanserver.org
 * Copyright (C) Daniele Affinita
 * Copyright (C) Chen Daye
 */


#ifndef TAN_HTTP_H
#define TAN_HTTP_H


#include "tan_core.h"

#include <string>


std::string tan_http_header_get_value(const char *header,
                                      const char *field);


#endif /* TAN_HTTP_H */

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_JSON_H
#define TAN_JSON_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


/**
 * Append status code and message to a JSON string.
 *
 * @param json_string: The JSON string to be processed
 * @param stat:        The specified status code
 * @param msg:         The specified message
 *
 * @return For example: {"status":200,"message":"OK","result":{$json_string}}
 */
TAN_PUBLIC_API const char *json_append_status(const char *json_string,
                                              const char *stat,
                                              const char *msg);


#ifdef __cplusplus
}
#endif


#endif /* TAN_JSON_H */

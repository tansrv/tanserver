/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_USER_API_H
#define TAN_USER_API_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


/**
 * Import 'user_api' module.
 *
 * @return TAN_OK || TAN_ERROR
 */
tan_int_t tan_user_api_init();

/**
 * Call the specified user API and pass it a JSON object.
 *
 * @param func:     The specified function name
 * @param json_obj: The JSON object to be passed
 *
 * @return If the specified function does not exist, it
 *         will return NULL, otherwise it will return PyObject *.
 */
PyObject *tan_call_user_api(const char *func, PyObject *json_obj);

void tan_reload_user_api();


#ifdef __cplusplus
}
#endif


#endif /* TAN_USER_API_H */

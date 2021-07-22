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
 * @param addr:     Hostaddr
 *
 * @return PyObject * || NULL
 * 
 * @note PyUnicode_AsUTF8() should be called to convert it to a string.
 */
PyObject *tan_call_user_api(const char *func, PyObject *json_obj,
                            const u_char *addr);

void tan_reload_user_api();


#ifdef __cplusplus
}
#endif


#endif /* TAN_USER_API_H */

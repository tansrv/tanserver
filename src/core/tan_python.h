/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_PYTHON_H
#define TAN_PYTHON_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"

#include <python2.7/Python.h>


/**
 * Initialize the Python interpreter and import
 * the "json" and "user_api" modules.
 *
 * @return TAN_OK || TAN_ERROR
 */
tan_int_t tan_py_env_init();

/**
 * Call json.loads() to convert a JSON string to PyObject.
 *
 * @param json_string: The JSON string to be converted
 * 
 * @return Invalid JSON string will return NULL,
 *         otherwise it will return PyObject *.
 */
PyObject *tan_py_json_string_to_object(const char *json_string);

/**
 * Call the specified Python function and pass a PyObject.
 *
 * @param func:   Python function name
 * @param object: The PyObject passed to the Python function
 *
 * @return The string returned by the Python function.
 */
char *tan_py_call_function(const char *func, PyObject *object);


#ifdef __cplusplus
}
#endif


#endif /* TAN_PYTHON_H */

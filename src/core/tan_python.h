/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_PYTHON_H
#define TAN_PYTHON_H


#include "tan_core.h"

#include <Python.h>


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Initialize the Python interpreter and set sys.path.
 */
void tan_py_env_init();

/**
 * Call json.loads() to convert a JSON string to PyObject.
 *
 * @param json_string: The JSON string to be converted
 * 
 * @return Invalid JSON string will return NULL,
 *         otherwise it will return PyObject *.
 */
PyObject *tan_py_json_string_to_object(const char *json_string);


#ifdef __cplusplus
}
#endif


#endif /* TAN_PYTHON_H */

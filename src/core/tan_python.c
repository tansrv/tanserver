/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_python.h"


#define TAN_PY_JSON_MODULE      "json"
#define TAN_PY_USER_API_MODULE  "user_api"


static tan_int_t tan_py_func_json_loads_init();
static tan_int_t tan_py_import_user_api_module();


static PyObject  *py_func_json_loads;
static PyObject  *user_api_module;


tan_int_t
tan_py_env_init()
{
    tan_int_t  ret;

    Py_Initialize();

    ret = tan_py_func_json_loads_init();
    if (ret != TAN_OK)
        return ret;

    return tan_py_import_user_api_module();
}


static tan_int_t
tan_py_func_json_loads_init()
{
    PyObject  *name, *module;

    name = PyString_FromString(TAN_PY_JSON_MODULE);
    if (name == NULL) {

        tan_stderr_error(0, "PyString_FromString(\"%s\") failed",
                         TAN_PY_JSON_MODULE);

        return TAN_ERROR;
    }

    module = PyImport_Import(name);
    if (module == NULL) {
        Py_DECREF(name);

        tan_stderr_error(0, "PyImport_Import(\"%s\") failed",
                         TAN_PY_JSON_MODULE);

        return TAN_ERROR;
    }

    Py_DECREF(name);

    py_func_json_loads = PyObject_GetAttrString(module, "loads");
    if (py_func_json_loads == NULL) {
        Py_DECREF(module);

        tan_stderr_error(0, "PyObject_GetAttrString(\"%s\", \"loads\") failed",
                         TAN_PY_JSON_MODULE);

        return TAN_ERROR;
    }

    Py_DECREF(module);
    return TAN_OK;
}


static tan_int_t
tan_py_import_user_api_module()
{
    PyObject  *name, *module;

    name = PyString_FromString(TAN_PY_USER_API_MODULE);
    if (name == NULL) {

        tan_stderr_error(0, "PyString_FromString(\"%s\") failed",
                         TAN_PY_USER_API_MODULE);

        return TAN_ERROR;
    }

    user_api_module = PyImport_Import(name);
    if (user_api_module == NULL) {
        Py_DECREF(name);

        tan_stderr_error(0, "PyImport_Import(\"%s\") failed",
                         TAN_PY_USER_API_MODULE);

        return TAN_ERROR;
    }

    Py_DECREF(name);
    return TAN_OK;
}


PyObject *
tan_py_json_string_to_object(const char *json_string)
{
    PyObject  *arg, *res;

    /* z: string  */
    arg = Py_BuildValue("(z)", json_string);
    if (arg == NULL) {

        tan_log_info("Py_BuildValue(\"(z)\") failed", NULL);
        return NULL;
    }

    res = PyObject_CallObject(py_func_json_loads, arg);
    Py_DECREF(arg);

    return res;
}


PyObject *
tan_py_call_function(const char *func, PyObject *object)
{
    PyObject  *py_func, *res;

    py_func = PyObject_GetAttrString(user_api_module, func);
    if (py_func == NULL)
        return NULL;

    res = PyObject_CallObject(py_func, object);

    Py_DECREF(py_func);
    return res;
}

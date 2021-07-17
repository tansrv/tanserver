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

    PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('/usr/local/tanserver/lib/')");

    return tan_py_import_user_api_module();
}


static tan_int_t
tan_py_func_json_loads_init()
{
    PyObject  *module;

    module = PyImport_ImportModule(TAN_PY_JSON_MODULE);
    if (module == NULL) {

        tan_stderr_error(0, "PyImport_Import(\"%s\") failed",
                         TAN_PY_JSON_MODULE);

        return TAN_ERROR;
    }

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
    system("find /usr/local/tanserver/user_api/ -name '*.py' | "
           "xargs paste -s -d '\n' > "
           "/usr/local/tanserver/lib/user_api.py");

    user_api_module = PyImport_ImportModule(TAN_PY_USER_API_MODULE);
    if (user_api_module == NULL) {

        tan_log_crit(0, "PyImport_Import(\"%s\") failed",
                     TAN_PY_USER_API_MODULE);

        return TAN_ERROR;
    }

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
    PyObject  *arg, *py_func, *res;

    arg = Py_BuildValue("(O)", object);
    if (arg == NULL)
        return NULL;

    py_func = PyObject_GetAttrString(user_api_module, func);
    if (py_func == NULL) {

        Py_DECREF(arg);
        return NULL;
    }

    res = PyObject_CallObject(py_func, arg);

    Py_DECREF(py_func);
    Py_DECREF(arg);

    return res;
}


tan_int_t
tan_py_re_import_user_api_module()
{
    system("find /usr/local/tanserver/user_api/ -name '*.py' | "
           "xargs paste -s -d '\n' > "
           "/usr/local/tanserver/lib/user_api.py");

    user_api_module = PyImport_ReloadModule(user_api_module);
    if (user_api_module == NULL) {

        tan_log_crit(0, "PyImport_ReloadModule(\"%s\") failed",
                     TAN_PY_USER_API_MODULE);

        return TAN_ERROR;
    }

    return TAN_OK;
}

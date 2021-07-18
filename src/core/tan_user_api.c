/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_user_api.h"


#define TAN_USER_API_MODULE  "user_api"


static void tan_user_api_merge_py_file();


static PyObject  *user_api_module;


tan_int_t
tan_user_api_init()
{
    tan_user_api_merge_py_file();

    user_api_module = PyImport_ImportModule(TAN_USER_API_MODULE);
    if (user_api_module == NULL) {
        PyErr_Print();

        tan_stderr_error(0, "PyImport_ImportModule(\"%s\") failed",
                         TAN_USER_API_MODULE);

        return TAN_ERROR;
    }

    return TAN_OK;
}


static void
tan_user_api_merge_py_file()
{
    system("find /usr/local/tanserver/user_api/ -name '*.py' | "
           "xargs paste -s -d '\n' > "
           "/usr/local/tanserver/lib/user_api.py");
}


PyObject *
tan_call_user_api(const char *func, PyObject *json_obj)
{
    int        reload;
    PyObject  *arg, *tmp, *py_func, *res;

    arg = Py_BuildValue("(O)", json_obj);
    if (arg == NULL) {

        tan_log_crit(0, "Py_BuildValue(\"(O)\") failed", NULL);
        return NULL;
    }

    tmp    = user_api_module;
    reload = 1;

    user_api_module = PyImport_ReloadModule(user_api_module);
    if (user_api_module == NULL) {

        tan_log_crit(0, "PyImport_ReloadModule(\"%s\") failed",
                     TAN_USER_API_MODULE);

        reload          = 0;
        user_api_module = tmp;
    }

    if (reload)
        Py_DECREF(tmp);

    py_func = PyObject_GetAttrString(user_api_module, func);
    if (py_func == NULL) {

        Py_DECREF(arg);
        return NULL;
    }

    res = PyObject_CallObject(py_func, arg);

    Py_DECREF(arg);
    Py_DECREF(py_func);

    return res;
}


void
tan_reload_user_api()
{
    tan_user_api_merge_py_file();
}

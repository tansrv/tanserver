/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_python.h"


#define TAN_PY_JSON_MODULE  "json"


void
tan_py_env_init()
{
    Py_Initialize();

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('/usr/local/tanserver/lib/')");
}


PyObject *
tan_py_json_string_to_object(const char *json_string)
{
    PyObject  *arg, *module, *func, *res;

    arg = Py_BuildValue("(z)", json_string);
    if (arg == NULL) {

        tan_log_crit(0, "Py_BuildValue(\"(z)\") failed", NULL);
        return NULL;
    }

    module = PyImport_ImportModule(TAN_PY_JSON_MODULE);
    if (module == NULL) {

        tan_log_crit(0, "PyImport_Import(\"%s\") failed",
                     TAN_PY_JSON_MODULE);

        Py_DECREF(arg);
        return NULL;
    }

    func = PyObject_GetAttrString(module, "loads");
    if (func == NULL) {

        tan_log_crit(0, "PyObject_GetAttrString(\"%s\", \"loads\") failed",
                     TAN_PY_JSON_MODULE);

        Py_DECREF(arg);
        Py_DECREF(module);

        return NULL;
    }

    res = PyObject_CallObject(func, arg);

    Py_DECREF(arg);
    Py_DECREF(module);
    Py_DECREF(func);

    return res;
}

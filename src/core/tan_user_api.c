/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_user_api.h"


static tan_int_t tan_user_api_merge_py_file();
static void tan_get_user_api_module_name(char *buf);


static PyObject  *user_api_module;


tan_int_t
tan_load_user_api_module()
{
    char       module_name[TAN_MAX_STR_SIZE];
    tan_int_t  ret;

    ret = tan_user_api_merge_py_file();
    if (ret != TAN_OK)
        return ret;

    tan_memzero(module_name, TAN_MAX_STR_SIZE);
    tan_get_user_api_module_name(module_name);

    user_api_module = PyImport_ImportModule(module_name);
    if (user_api_module == NULL) {
        PyErr_Print();

        tan_stderr_error(0, "PyImport_ImportModule(\"%s\") failed",
                         module_name);

        tan_remove_user_api_module();
        return TAN_ERROR;
    }

    return TAN_OK;
}


static tan_int_t
tan_user_api_merge_py_file()
{
    char  buf[TAN_MAX_STR_SIZE];

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    snprintf(buf, TAN_MAX_STR_SIZE,
             "find /usr/local/tanserver/user_api/ -name '*.py' | "
             "xargs paste -s -d '\n' > "
             "/usr/local/tanserver/lib/user_api_%d.py",
             getpid());

    if (system(buf) != -1)
        return TAN_OK;

    tan_log_crit(errno, "system(\"%s\") failed", buf);
    return TAN_ERROR;
}


static void
tan_get_user_api_module_name(char *buf)
{
    snprintf(buf, TAN_MAX_STR_SIZE, "user_api_%d", getpid());
}


PyObject *
tan_call_user_api(const char *func, PyObject *json_obj,
                  const u_char *addr)
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
        tan_log_info("PyImport_ReloadModule(\"user_api\") failed", NULL);

        reload          = 0;
        user_api_module = tmp;
    }

    if (reload)
        Py_DECREF(tmp);

    py_func = PyObject_GetAttrString(user_api_module, func);
    if (py_func == NULL) {

        tan_log_info(TAN_CUSTOM_PROTOCOL_ERROR_FUNCTION_NOT_FOUND,
                     func,
                     addr[0], addr[1], addr[2], addr[3]);

        Py_DECREF(arg);
        return NULL;
    }

    res = PyObject_CallObject(py_func, arg);
    if (res == NULL)
        tan_log_warn(0, "PyObject_CallObject(\"%s\") failed", func);

    Py_DECREF(arg);
    Py_DECREF(py_func);

    return res;
}


void
tan_reload_user_api_module()
{
    tan_user_api_merge_py_file();
}


void
tan_remove_user_api_module()
{
    char  buf[TAN_MAX_STR_SIZE];

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    snprintf(buf, TAN_MAX_STR_SIZE,
             "/usr/local/tanserver/lib/user_api_%d.py",
             getpid());

    if (remove(buf))
        tan_log_warn(errno, "remove(\"%s\") failed", buf);
}

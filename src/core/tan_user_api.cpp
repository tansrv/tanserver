/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_user_api.h"


#define TAN_USER_API_SO_FILE \
    TAN_INSTALL_PATH"lib/libuser_api.so"


static std::string tan_get_user_api_so_filename();
static tan_int_t tan_so_file_copy(const char *src,
                                  const char *dest);


static void  *dl;


tan_int_t
tan_load_user_api()
{
    tan_int_t    ret;
    std::string  filename;

    filename = tan_get_user_api_so_filename();

    ret = tan_so_file_copy(TAN_USER_API_SO_FILE,
                           filename.c_str());

    if (ret != TAN_OK)
        return ret;

    dl = dlopen(filename.c_str(), RTLD_NOW);
    if (dl == NULL) {
        tan_log_crit(0, "dlopen(\"%s\") failed", dlerror());

        remove(filename.c_str());
        return TAN_ERROR;
    }

    return TAN_OK;
}


static std::string
tan_get_user_api_so_filename()
{
    char  buf[TAN_PATH_SIZE];

    tan_memzero(buf, TAN_PATH_SIZE);

    snprintf(buf, TAN_PATH_SIZE,
             TAN_INSTALL_PATH"lib/libuser_api_%d.so",
             getpid());

    return std::string(buf);
}


static tan_int_t
tan_so_file_copy(const char *src, const char *dest)
{
    return tan_file_copy(src, dest);
}


tan_user_api_t
tan_get_user_api_handler(const char *func)
{
    return (tan_user_api_t)dlsym(dl, func);
}


tan_int_t
tan_reload_user_api()
{
    tan_unload_user_api();

    return tan_load_user_api();
}


void
tan_unload_user_api()
{
    std::string  filename;

    if (dlclose(dl)) {
        tan_log_warn(0, "dlclose(\"%s\") failed",
                     dlerror());
    }

    filename = tan_get_user_api_so_filename();

    if (remove(filename.c_str())) {
        tan_log_warn(errno, "remove(\"%s\") failed",
                     filename.c_str());
    }
}

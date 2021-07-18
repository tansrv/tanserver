/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_json.h"

#include <string>


static std::string  res;


const char *
json_append_status(const char *json_string,
                   const char *stat,
                   const char *msg)
{
    char  buf[TAN_MAX_STR_SIZE];

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    snprintf(buf, TAN_MAX_STR_SIZE,
             "{\"status\":%s,\"message\":\"%s\",\"result\":",
             stat, msg);

    res = buf;

    res.append(json_string);
    res.append("}");

    return res.c_str();
}

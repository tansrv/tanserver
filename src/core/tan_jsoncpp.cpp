/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_jsoncpp.h"


std::string
json_encode(const Json::Value &value)
{
    Json::FastWriter  writer;

    writer.omitEndingLineFeed();

    return writer.write(value);
}


Json::Value
json_decode(const std::string &json_string)
{
    Json::Value   value;
    Json::Reader  reader;

    if (reader.parse(json_string, value) == false)
        throw "error";

    return value;
}


void
json_append_status(std::string &json_string,
                   int stat, const char *msg)
{
    char  buf[TAN_MAX_STR_SIZE];

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    snprintf(buf, TAN_MAX_STR_SIZE,
             "{\"status\":%d,\"message\":\"%s\",\"result\":",
             stat, msg);

    json_string.insert(0, buf);
    json_string.append("}");
}

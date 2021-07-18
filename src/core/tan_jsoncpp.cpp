/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
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

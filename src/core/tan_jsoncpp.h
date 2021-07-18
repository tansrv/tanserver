/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_JSONCPP_H
#define TAN_JSONCPP_H


#include <json.h>
#include <string>


std::string json_encode(const Json::Value &value);
Json::Value json_decode(const std::string &json_string);


#endif /* TAN_JSONCPP_H */

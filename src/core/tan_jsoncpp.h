/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_JSONCPP_H
#define TAN_JSONCPP_H


#include <json.h>


std::string json_encode(const Json::Value &value);
Json::Value json_decode(const std::string &json_string);

/**
 * Append status code and message to JSON string
 *
 * @param json_string: JSON string
 * @param stat: Status code
 * @param msg: Message
 *
 * @see user_api/examples.cpp - append_status()
 */
void json_append_status(std::string &json_string,
                        int stat, const char *msg);


#endif /* TAN_JSONCPP_H */

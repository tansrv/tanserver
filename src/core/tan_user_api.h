/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_USER_API_H
#define TAN_USER_API_H


#include "tan_core.h"

#include <json.h>
#include <string>


typedef std::string (*tan_user_api_t)(const Json::Value &);


tan_int_t tan_load_user_api();
tan_user_api_t tan_get_user_api_handler(const char *func);
tan_int_t tan_reload_user_api();
void tan_unload_user_api();


#endif /* TAN_USER_API_H */

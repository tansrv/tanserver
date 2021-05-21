/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_TIME_H
#define TAN_TIME_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


void tan_time_update();
const char *tan_get_localtime_str();
time_t tan_get_time();
void tan_sec_to_day(time_t sec, struct tm *tm);


#define tan_msleep(ms)  usleep(ms * 1000)


#ifdef __cplusplus
}
#endif


#endif /* TAN_TIME_H */

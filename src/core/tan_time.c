/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_time.h"


#define TAN_LOCALTIME_STR_SIZE  30


static struct {
    char    localtime_str[TAN_LOCALTIME_STR_SIZE];
    time_t  now;
} cached_time;


void
tan_time_update()
{
    struct tm  *tm;

    cached_time.now = time(NULL);

    tm = localtime(&cached_time.now);

    tan_memzero(cached_time.localtime_str,
                TAN_LOCALTIME_STR_SIZE);

    snprintf(cached_time.localtime_str,
             TAN_LOCALTIME_STR_SIZE,
             "%d/%02d/%02d %02d:%02d:%02d",
             tm->tm_year + 1900, tm->tm_mon + 1,
             tm->tm_mday, tm->tm_hour,
             tm->tm_min, tm->tm_sec);
}


const char *
tan_get_localtime_str()
{
    return cached_time.localtime_str;
}


time_t
tan_get_time()
{
    return cached_time.now;
}


void
tan_sec_to_day(time_t sec, struct tm *tm)
{
    tm->tm_mday = sec / 86400;
    tm->tm_hour = (sec % 86400) / 3600;
    tm->tm_min  = (sec % 86400 % 3600) / 60;
    tm->tm_sec  = sec % 86400 % 3600 % 60;
}

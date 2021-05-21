/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#ifndef TAN_POSTGRESQL_H
#define TAN_POSTGRESQL_H


#include <string>


int tan_pgconn_init();
std::string tan_pg_query(const char *func, const char *hostaddr,
                         const char *query, ...);
void tan_pgconn_free();


/**
 * Query database and get the first field of the first row
 *
 * @param hostaddr: Database server IP address (/etc/tanserver.conf)
 * @param query: SQL statement, use $1, $2, $3... to replace variable arguments
 * @param ...: Variable arguments, only supports C-style strings
 *
 * @return The first field of the first row or an empty string
 *
 * @see user_api/examples.cpp - 
 *      login_register() / get_items() / transaction()
 */
#define pg_query(hostaddr, query, ...) \
   tan_pg_query(__func__, hostaddr, query, __VA_ARGS__)


#endif /* TAN_POSTGRESQL_H */

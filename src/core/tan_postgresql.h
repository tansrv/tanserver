/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_POSTGRESQL_H
#define TAN_POSTGRESQL_H


#include <string>


int tan_pgconn_init();

/**
 * Query database and get the first field of the first row.
 *
 * @param hostaddr: Database server IP address (/etc/tanserver.conf)
 * @param query:    SQL statement, use $1, $2, $3... to replace variable arguments
 * @param ...:      Variable arguments, only supports C-style strings
 *
 * @return The first field of the first row or an empty string.
 */
std::string pg_query(const char *hostaddr, const char *query, ...);

void tan_pgconn_free();


#endif /* TAN_POSTGRESQL_H */

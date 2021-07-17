/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_POSTGRESQL_H
#define TAN_POSTGRESQL_H


#ifdef __cplusplus
extern "C" {
#endif


int tan_pgconn_init();
void tan_pgconn_free();

/**
 * Query database and get the first field of the first row.
 *
 * @param hostaddr: Database server IP address (/etc/tanserver.conf)
 * @param query:    SQL statement, use $1, $2, $3... to replace variable arguments
 * @param ...:      Variable arguments, only supports C-style strings
 *
 * @return The first field of the first row or an empty string.
 */
const char *pg_query(const char *hostaddr,
                     const char *query, ...);


#ifdef __cplusplus
}
#endif


#endif /* TAN_POSTGRESQL_H */

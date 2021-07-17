/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_events.h"

#if (TAN_HAVE_REDHAT_PG_HEADER)
#include <libpq-fe.h>
#else
#include <postgresql/libpq-fe.h>
#endif

#include <string>


#define TAN_MAX_PARAM_COUNT  128


typedef struct {
    char     hostaddr[TAN_HOSTADDR_SIZE];
    PGconn  *conn;
} tan_pgconn_t;


tan_vector_t(pgconn, tan_pgconn_t *);


static int tan_pg_connect(tan_pgconn_t *conn, tan_pg_cfg_t *cfg);
static void tan_pg_conninfo_init(char *buf, tan_pg_cfg_t *cfg);

static PGconn *tan_get_pgconn(const char *func, const char *hostaddr);
static int tan_get_pgconn_index(const char *hostaddr);

static tan_int_t tan_pg_exec_params(const char *func, PGconn *conn,
                                    const char *query, va_list arg);
static int tan_get_param_count(const char *query);
static void tan_check_pgconn_status(PGconn *conn);


static std::string          query_res;
static tan_vector_pgconn_t  conns;


int
tan_pgconn_init()
{
    int            k;
    void          *mem;
    tan_pgconn_t  *conn;

    mem = calloc(tan_get_pg_cfg()->size, sizeof(tan_pgconn_t));
    if (mem == NULL) {

        tan_log_crit(errno, "calloc() failed", NULL);
        return -1;
    }

    tan_vector_init(conns);

    for (k = 0; k < tan_get_pg_cfg()->size; ++k) {

        conn = (tan_pgconn_t *)((char *)mem +
                sizeof(tan_pgconn_t) * k);

        if (tan_pg_connect(conn, tan_get_pg_cfg()->vec[k])) {

            free(mem);
            return -1;
        }

        strcpy(conn->hostaddr,
               tan_get_pg_cfg()->vec[k]->hostaddr);

        tan_vector_push_back(conns, tan_pgconn_t *, conn);
    }

    return 0;
}


static int
tan_pg_connect(tan_pgconn_t *conn, tan_pg_cfg_t *cfg)
{
    char  buf[TAN_MAX_STR_SIZE];

    tan_memzero(buf, TAN_MAX_STR_SIZE);

    tan_pg_conninfo_init(buf, cfg);

    conn->conn = PQconnectdb(buf);

    if (PQstatus(conn->conn) == CONNECTION_BAD) {
        tan_log_database(NULL, "%s", PQerrorMessage(conn->conn));

        PQfinish(conn->conn);
        return -1;
    }

    return 0;
}


static void
tan_pg_conninfo_init(char *buf, tan_pg_cfg_t *cfg)
{
    snprintf(buf, TAN_MAX_STR_SIZE,
             "hostaddr=%s port=%u "
             "dbname=%s user=%s "
             "password=%s connect_timeout=2",
             cfg->hostaddr, cfg->port,
             cfg->database, cfg->user, cfg->password);
}


const char *
pg_query(const char *hostaddr, const char *query, ...)
{
    va_list      arg;
    PGconn      *conn;
    const char  *func;

    /* Get the API name.  */
    func = tan_get_current_connection()->event.user_api.c_str();

    conn = tan_get_pgconn(func, hostaddr);
    if (conn == NULL)
        return NULL;

    va_start(arg, query);

    /*
     * If it returns TAN_OK, it means query_res is set.
     * Otherwise, NULL should be returned, so that Python
     * gets 'None' and throws an exception to the user.
     */
    if (tan_pg_exec_params(func, conn, query, arg) != TAN_OK)
        return NULL;

    va_end(arg);

    return query_res.c_str();
}


static PGconn *
tan_get_pgconn(const char *func, const char *hostaddr)
{
    int  k;

    k = tan_get_pgconn_index(hostaddr);
    if (k == -1) {

        tan_log_database_lf(func, "invalid hostaddr: %s",
                            hostaddr);

        return NULL;
    }

    return conns.vec[k]->conn;
}


static int
tan_get_pgconn_index(const char *hostaddr)
{
    int  k;

    for (k = 0; k < conns.size; ++k) {

        if (!strcmp(conns.vec[k]->hostaddr, hostaddr))
            return k;
    }

    return -1;
}


static tan_int_t
tan_pg_exec_params(const char *func, PGconn *conn,
                   const char *query, va_list arg)
{
    int            k, param_count;
    PGresult      *res;
    const char    *param_values[TAN_MAX_PARAM_COUNT];
    const u_char  *p;

    param_count = tan_get_param_count(query);
    if (tan_unlikely(param_count > TAN_MAX_PARAM_COUNT)) {

        tan_log_database_lf(func, "maximum number of parameters: %d",
                            TAN_MAX_PARAM_COUNT);

        return TAN_ERROR;
    }

    for (k = 0; k < param_count; ++k)
        param_values[k] = va_arg(arg, const char *);

    res = PQexecParams(conn, query, param_count,
                       NULL, param_values, NULL, NULL, 0);

    k = PQresultStatus(res);

    if (k != PGRES_TUPLES_OK &&
        k != PGRES_COMMAND_OK)
    {
        p = tan_get_hostaddr(&tan_get_current_connection()->info.addr);

        tan_log_database(func, "client: %d.%d.%d.%d, %s",
                         p[0], p[1], p[2], p[3],
                         PQerrorMessage(conn));

        tan_check_pgconn_status(conn);

        PQclear(res);
        return TAN_ERROR;
    }

    if (!PQntuples(res)) {
        PQclear(res);

        query_res = "";
        return TAN_OK;
    }

    query_res = std::string(PQgetvalue(res, 0, 0));

    PQclear(res);
    return TAN_OK;
}


static int
tan_get_param_count(const char *query)
{
    int  count, k;

    count = 0;

    for (k = 0; query[k]; ++k) {

        if (query[k] == '$' &&
            query[k + 1] >= '1' && query[k + 1] <= '9')
        {
            ++count;
        }
    }

    return count;
}


static void
tan_check_pgconn_status(PGconn *conn)
{
    if (PQstatus(conn) == CONNECTION_BAD)
        PQreset(conn);
}


void
tan_pgconn_free()
{
    int  k;

    for (k = 0; k < conns.size; ++k)
        PQfinish(conns.vec[k]->conn);
}

/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_config.h"

#include <confuse.h>


#define TAN_DEFAULT_LISTEN_PORT           2579
#define TAN_DEFAULT_CLIENT_MAX_BODY_SIZE  16384
#define TAN_DEFAULT_REQUEST_TIMEOUT       60

#define TAN_DEFAULT_PGSQL_HOSTADDR        "127.0.0.1"
#define TAN_DEFAULT_PGSQL_LISTEN_PORT     5432

#define TAN_DEFAULT_LOG_DIRECTORY         "/var/log/tanserver/"

#define TAN_DEFAULT_SHIPPER_LISTEN_PORT   1117
#define TAN_DEFAULT_SHIPPER_HOSTNAME      "tanserver"


static cfg_t *tan_cfg_init();
static void tan_load_server(cfg_t *cfg);
static tan_int_t tan_load_ssl(cfg_t *cfg);
static tan_int_t tan_check_ssl_file(const char *cert,
                                    const char *key);
static tan_int_t tan_load_database(cfg_t *cfg);
static tan_int_t tan_load_pg(cfg_t *cfg);
static tan_int_t tan_check_db(cfg_t *cfg, const char *db,
                              const char *str);
static tan_int_t tan_load_log(cfg_t *cfg);
static tan_int_t tan_load_log_shipper(cfg_t *cfg);
static tan_int_t tan_validate_hostname(const char *name);
static tan_int_t tan_load_allowlist_to_vec(cfg_t *cfg);
static tan_int_t tan_verify_hostaddr(const char *hostaddr);
static void tan_cfg_error_print(const char *str);


static struct {
    tan_server_cfg_t     server;
    tan_ssl_cfg_t        ssl;
    tan_vector_pg_cfg_t  pg;
    tan_log_cfg_t        log;
} config;


static cfg_t *
tan_cfg_init()
{
    cfg_t  *cfg;

/********************************server********************************/

    cfg_opt_t  server[] = {
        CFG_INT("listen", TAN_DEFAULT_LISTEN_PORT, CFGF_NONE),
        CFG_INT("client_max_body_size", TAN_DEFAULT_CLIENT_MAX_BODY_SIZE, CFGF_NONE),
        CFG_INT("request_timeout", TAN_DEFAULT_REQUEST_TIMEOUT, CFGF_NONE),
        CFG_END()
    };

/*********************************ssl**********************************/

    cfg_opt_t  ssl[] = {
        CFG_STR("ssl_cert_file", "?", CFGF_NONE),
        CFG_STR("ssl_key_file", "?", CFGF_NONE),
        CFG_END()
    };

/*******************************database*******************************/

    cfg_opt_t  pg[] = {
        CFG_STR("hostaddr", TAN_DEFAULT_PGSQL_HOSTADDR, CFGF_NONE),
        CFG_INT("port", TAN_DEFAULT_PGSQL_LISTEN_PORT, CFGF_NONE),
        CFG_STR("user", "?", CFGF_NODEFAULT),
        CFG_STR("password", "?", CFGF_NODEFAULT),
        CFG_STR("database", "?", CFGF_NODEFAULT),
        CFG_END()
    };

    cfg_opt_t  database_sec[] = {
        CFG_SEC("pgsql", pg, CFGF_MULTI),
        CFG_END()
    };

/*********************************log**********************************/

    cfg_opt_t  shipper[] = {
        CFG_INT("listen", TAN_DEFAULT_SHIPPER_LISTEN_PORT, CFGF_NONE),
        CFG_STR("hostname", TAN_DEFAULT_SHIPPER_HOSTNAME, CFGF_NONE),
        CFG_STR_LIST("allowlist", "{}", CFGF_NODEFAULT),
        CFG_END()
    };

    cfg_opt_t  log[] = {
        CFG_STR("directory", TAN_DEFAULT_LOG_DIRECTORY, CFGF_NONE),
        CFG_SEC("shipper", shipper, CFGF_NO_TITLE_DUPES),
        CFG_END()
    };

/*********************************end**********************************/

    cfg_opt_t  sec[] = {
        CFG_SEC("server", server, CFGF_NO_TITLE_DUPES),
        CFG_SEC("ssl", ssl, CFGF_NO_TITLE_DUPES),
        CFG_SEC("database", database_sec, CFGF_NO_TITLE_DUPES),
        CFG_SEC("log", log, CFGF_NO_TITLE_DUPES),
        CFG_END()
    };

    return cfg_init(sec, CFGF_NONE);
}


tan_int_t
tan_load_cfg()
{
    cfg_t     *cfg;
    tan_int_t  ret;

    cfg = tan_cfg_init();

    if (cfg_parse(cfg, TAN_CFG_PATH) != CFG_SUCCESS)
        return TAN_ERROR;

    tan_memzero(&config, sizeof(config));

    tan_load_server(cfg);

    ret = tan_load_ssl(cfg);
    if (ret != TAN_OK)
        goto out;

    ret = tan_load_database(cfg);
    if (ret != TAN_OK)
        goto out;

    ret = tan_load_log(cfg);

out:

    cfg_free(cfg);
    return ret;
}


static void
tan_load_server(cfg_t *cfg)
{
    cfg_t  *sec;

    sec = cfg_getsec(cfg, "server");

    config.server.port = (in_port_t)cfg_getint(sec, "listen");

    config.server.client_max_body_size = cfg_getint(sec, "client_max_body_size");
    if (config.server.client_max_body_size <= 0)
        config.server.client_max_body_size = TAN_DEFAULT_CLIENT_MAX_BODY_SIZE;

    config.server.request_timeout = cfg_getint(sec, "request_timeout");
    if (config.server.request_timeout <= 0)
        config.server.request_timeout = TAN_DEFAULT_REQUEST_TIMEOUT;
}


static tan_int_t
tan_load_ssl(cfg_t *cfg)
{
    cfg_t     *sec;
    tan_int_t  ret;

    sec = cfg_getsec(cfg, "ssl");

    snprintf(config.ssl.ssl_cert_file, TAN_PATH_SIZE,
             "%s", cfg_getstr(sec, "ssl_cert_file"));

    snprintf(config.ssl.ssl_key_file, TAN_PATH_SIZE,
             "%s", cfg_getstr(sec, "ssl_key_file"));

    ret = tan_check_ssl_file(config.ssl.ssl_cert_file,
                             config.ssl.ssl_key_file);

    if (ret != TAN_OK)
        tan_cfg_error_print("ssl: missing cert or key file");

    return ret;
}


static tan_int_t
tan_check_ssl_file(const char *cert, const char *key)
{
    if (tan_check_file_exists(cert) == -1 ||
        tan_check_file_exists(key) == -1)
    {
        return TAN_ERROR;
    }

    return TAN_OK;
}


static tan_int_t
tan_load_database(cfg_t *cfg)
{
    cfg_t  *sec;

    sec = cfg_getsec(cfg, "database");

    return tan_load_pg(sec);
}


static tan_int_t
tan_load_pg(cfg_t *cfg)
{
    cfg_t         *sec;
    unsigned       count, k;
    tan_pg_cfg_t  *pg;

    tan_vector_init(config.pg);

    count = cfg_size(cfg, "pgsql");
    if (!count)
        return TAN_OK;

    for (k = 0; k < count; ++k) {

        sec = cfg_getnsec(cfg, "pgsql", k);

        if (tan_check_db(sec, "pgsql", "user") != TAN_OK ||
            tan_check_db(sec, "pgsql", "password") != TAN_OK ||
            tan_check_db(sec, "pgsql", "database") != TAN_OK)
        {
            return TAN_ERROR;
        }

        pg = (tan_pg_cfg_t *)calloc(1, sizeof(tan_pg_cfg_t));
        if (pg == NULL) {

            tan_stderr_error(errno, "calloc() failed");
            return TAN_ERROR;
        }

        snprintf(pg->hostaddr, TAN_HOSTADDR_SIZE,
                 "%s", cfg_getstr(sec, "hostaddr"));

        pg->port = (in_port_t)cfg_getint(sec, "port");

        snprintf(pg->user, TAN_PG_IDENTIFIER_SIZE,
                 "%s", cfg_getstr(sec, "user"));

        snprintf(pg->password, TAN_PG_IDENTIFIER_SIZE,
                 "%s", cfg_getstr(sec, "password"));

        snprintf(pg->database, TAN_PG_IDENTIFIER_SIZE,
                 "%s", cfg_getstr(sec, "database"));

        tan_vector_push_back(config.pg, tan_pg_cfg_t *, pg);
    }

    return TAN_OK;
}


static tan_int_t
tan_check_db(cfg_t *cfg, const char *db,
             const char *str)
{
    if (cfg_getstr(cfg, str) == NULL) {

        tan_stderr_error(0, "invalid configuration file: "
                         "%s: missing '%s'", db, str);

        return TAN_ERROR;
    }

    return TAN_OK;
}


static tan_int_t
tan_load_log(cfg_t *cfg)
{
    cfg_t  *sec;

    sec = cfg_getsec(cfg, "log");

    snprintf(config.log.directory, TAN_PATH_SIZE,
             "%s", cfg_getstr(sec, "directory"));

    if (tan_check_file_exists(config.log.directory)
        == -1)
    {
        if (mkdir(config.log.directory, 0700)) {

            tan_stderr_error(errno, "mkdir(\"%s\") failed",
                             config.log.directory);

            return TAN_ERROR;
        }
    }

    return tan_load_log_shipper(sec);
}


static tan_int_t
tan_load_log_shipper(cfg_t *cfg)
{
    cfg_t     *sec;
    tan_int_t  ret;

    sec = cfg_getsec(cfg, "shipper");

    config.log.shipper.port = (in_port_t)cfg_getint(sec, "listen");

    snprintf(config.log.shipper.hostname, TAN_HOSTNAME_SIZE,
             "%s", cfg_getstr(sec, "hostname"));

    ret = tan_validate_hostname(config.log.shipper.hostname);
    if (ret != TAN_OK) {

        tan_cfg_error_print("shipper: illegal character in hostname");
        return ret;
    }

    tan_vector_init(config.log.shipper.allowlist);

    return tan_load_allowlist_to_vec(sec);
}


static tan_int_t
tan_validate_hostname(const char *name)
{
    int          k;
    const char  *ch = "\\<>()[]&:,/|?.*";

    for (k = 0; name[k]; ++k) {

        if (strchr(ch, name[k]) != NULL)
            return TAN_ERROR;
    }

    return TAN_OK;
}


static tan_int_t
tan_load_allowlist_to_vec(cfg_t *cfg)
{
    char      *str, *p;
    unsigned   count, k;
    tan_int_t  ret;

    count = cfg_size(cfg, "allowlist");

    for (k = 0; k < count; ++k) {

        str = cfg_getnstr(cfg, "allowlist", k);

        ret = tan_verify_hostaddr(str);
        if (ret != TAN_OK) {

            tan_cfg_error_print("allowlist: invalid ip address");
            return ret;
        }

        p = (char *)calloc(1, strlen(str) + 1);
        if (p == NULL) {

            tan_stderr_error(errno, "calloc() failed");
            return TAN_ERROR;
        }

        strcpy(p, str);

        tan_vector_push_back(config.log.shipper.allowlist,
                             char *, p);
    }

    return TAN_OK;
}


static tan_int_t
tan_verify_hostaddr(const char *hostaddr)
{
    struct in_addr  addr;

    if (inet_aton(hostaddr, &addr))
        return TAN_OK;

    return TAN_ERROR;
}


static void
tan_cfg_error_print(const char *str)
{
    tan_stderr_error(0, "invalid configuration file: %s", str);
}


tan_server_cfg_t *
tan_get_server_cfg()
{
    return &config.server;
}


tan_ssl_cfg_t *
tan_get_ssl_cfg()
{
    return &config.ssl;
}


tan_vector_pg_cfg_t *
tan_get_pg_cfg()
{
    return &config.pg;
}


tan_log_cfg_t *
tan_get_log_cfg()
{
    return &config.log;
}


int
tan_is_shipper()
{
    if (config.log.shipper.allowlist.size)
        return 1;

    return 0;
}


tan_shipper_cfg_t *
tan_get_shipper_cfg()
{
    return &config.log.shipper;
}

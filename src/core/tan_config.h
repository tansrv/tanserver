/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#ifndef TAN_CONFIG_H
#define TAN_CONFIG_H


#ifdef __cplusplus
extern "C" {
#endif


#include "tan_core.h"


typedef int  tan_int_t;

#define TAN_CFG_PATH      "/etc/tanserver.conf"
#define TAN_INSTALL_PATH  "/usr/local/tanserver/"

#define TAN_HOSTNAME_SIZE       100
#define TAN_HOSTADDR_SIZE       20
#define TAN_PATH_SIZE           1500
#define TAN_MAX_STR_SIZE        2048
#define TAN_PG_IDENTIFIER_SIZE  64


typedef struct {
    in_port_t            port;
    int                  client_max_body_size;
    int                  request_timeout;
} tan_server_cfg_t;


typedef struct {
    char                 ssl_cert_file[TAN_PATH_SIZE];
    char                 ssl_key_file[TAN_PATH_SIZE];
} tan_ssl_cfg_t;


typedef struct {
    char                 hostaddr[TAN_HOSTADDR_SIZE];
    in_port_t            port;

    char                 user[TAN_PG_IDENTIFIER_SIZE];
    char                 password[TAN_PG_IDENTIFIER_SIZE];

    char                 database[TAN_PG_IDENTIFIER_SIZE];
} tan_pg_cfg_t;


tan_vector_t(pg_cfg, tan_pg_cfg_t *);


typedef struct {
    in_port_t            port;
    char                 hostname[TAN_HOSTNAME_SIZE];
    tan_vector_string_t  allowlist;
} tan_shipper_cfg_t;


typedef struct {
    char                 directory[TAN_PATH_SIZE];
    tan_shipper_cfg_t    shipper;
} tan_log_cfg_t;


tan_int_t tan_load_cfg();
tan_server_cfg_t *tan_get_server_cfg();
tan_ssl_cfg_t *tan_get_ssl_cfg();
tan_vector_pg_cfg_t *tan_get_pg_cfg();
tan_log_cfg_t *tan_get_log_cfg();
int tan_is_shipper();
tan_shipper_cfg_t *tan_get_shipper_cfg();


#ifdef __cplusplus
}
#endif


#endif /* TAN_CONFIG_H */

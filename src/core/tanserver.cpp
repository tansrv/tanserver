/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 */


#include "tan_core.h"
#include "tan_openssl_ssl.h"
#include "tanserver.h"


#define TAN_INFO_HEADER      \
    "tanserver version "     \
    TANSERVER_VERSION " - "  \
    TANSERVER_DOCS


static int tan_getopt(int argc, char **argv);
static void tan_show_help();
static void tan_show_status();
static void tan_test_cfg();
static void tan_cfg_status_print(const char *str);
static void tan_show_version();


int
main(int argc, char **argv)
{
    if (tan_syscfg_init() != TAN_OK)
        return -1;

    if (tan_getopt(argc, argv))
        return 0;

    if (tan_load_cfg() != TAN_OK)
        return -1;

    if (tan_daemon() != TAN_OK)
        return -1;

    if (tan_signal_init() != TAN_OK)
        return -1;

    if (tan_log_init() != TAN_OK)
        return -1;

    if (tan_ssl_init() != TAN_OK)
        return -1;

    if (tan_create_pidfile() != TAN_OK)
        return -1;

    if (tan_status_shm_init() != TAN_OK)
        return -1;

    tan_py_env_init();

    tan_master_process_loop();

    return 0;
}


static int
tan_getopt(int argc, char **argv)
{
    int  ch;

    do {
        ch = getopt(argc, argv, "hatv");

        switch (ch) {

        case 'h':

            tan_show_help();
            return 1;

        case 'a':

            tan_show_status();
            return 1;

        case 't':

            tan_test_cfg();
            return 1;

        case 'v':

            tan_show_version();
            return 1;

        case '?':

            return -1;
        }
    } while (ch != -1);

    return 0;
}


static void
tan_show_help()
{
    tan_stderr_info(TAN_INFO_HEADER "\n\n"
                    "Usage\n  tanserver [-hatv]\n\n"
                    "Options\n"
                    "  -h: this help\n"
                    "  -a: show status\n"
                    "  -t: test configuration\n"
                    "  -v: show version\n");
}


static void
tan_show_status()
{
    struct tm  tm;

    if (tan_is_running() == -1) {

        tan_stderr_info(TAN_INFO_HEADER "\n"
                        "  Status: inactive");

        return;
    }

    if (tan_status_shm_attach(PROT_READ, MAP_SHARED)
        != TAN_OK)
    {
        return;
    }

    tan_sec_to_day(tan_get_uptime(), &tm);

    tan_stderr_info(TAN_INFO_HEADER "\n\n"
                    "  Status:             "
                    TAN_STRING_BRIGHT_GREEN "active" TAN_STRING_COLOR_END "\n"
                    "  Uptime:             %02dd - %02dh - %02dm - %02ds\n"
                    "  Active connections: %u\n"
                    "  Handled requests:   %llu\n",
                    tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec,
                    tan_get_active_connections(),
                    tan_get_handled_requests());

    tan_status_shm_munmap();
}


static void
tan_test_cfg()
{
    if (tan_load_cfg() != TAN_OK) {

        tan_cfg_status_print("test failed");
        return;
    }

    tan_cfg_status_print("test ok");
}


static void
tan_cfg_status_print(const char *str)
{
    tan_stderr_error(0, "configuration file: %s: %s",
                     TAN_CFG_PATH, str);
}


static void
tan_show_version()
{
    tan_stderr_info("tanserver version %s", TANSERVER_VERSION);
}

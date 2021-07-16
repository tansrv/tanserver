/*
 * Copyright (C) tanserver.org
 * Copyright (C) Chen Daye
 *
 * Feedback: tanserver@outlook.com
 */


#include "tan_core.h"
#include "tan_epoll.h"
#include "tan_events.h"
#include "tan_user_api.h"
#include "tan_postgresql.h"


#define TAN_MASTER_PROCESS   "tansrv-master"
#define TAN_WORKER_PROCESS   "tansrv-worker"
#define TAN_SHIPPER_PROCESS  "tansrv-shipper"


tan_vector_t(cpid, pid_t);


static tan_int_t tan_start_child_processes();
static void tan_worker_process_loop();
static tan_int_t tan_worker_process_init();
static void tan_worker_process_exit();
static void tan_notify_all_child_processes(int signo);
static int tan_reap_zombie_process();
static void tan_refork_child_process(int zpid);
static int tan_get_zpid_index(int zpid);
static void tan_release_resources();


static pid_t              ppid;
static int                reap;
static tan_socket_t       server;
static tan_vector_cpid_t  cpid;


void
tan_master_process_loop()
{
    int       signo;
    sigset_t  sigset;

    ppid = getpid();

    if (tan_start_child_processes() != TAN_OK)
        exit(-1);

    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR2);
    sigaddset(&sigset, SIGTERM);
    sigaddset(&sigset, SIGCHLD);

    if (sigprocmask(SIG_BLOCK, &sigset, NULL))
        tan_stderr_error(errno, "sigprocmask(SIG_BLOCK) failed");

    for (;;) {

        /*
         * Master process is waiting for a signal:
         *
         * SIGUSR2: Notify all worker processes to reload APIs.
         * SIGCHLD: Re-fork child process.
         * SIGTERM: Notify all child processes to exit.
         */
        if (sigwait(&sigset, &signo))
            tan_log_warn(errno, "sigwait() failed", NULL);

        /* Update the date string, which will be used to write the log.  */
        tan_time_update();

        switch (signo) {

        case SIGUSR2:

            tan_notify_all_child_processes(SIGUSR2);
            break;

        case SIGCHLD:

            tan_refork_child_process(tan_reap_zombie_process());
            break;

        case SIGTERM:

            tan_release_resources();
            tan_notify_all_child_processes(SIGUSR1);

            return;
        }
    }
}


static tan_int_t
tan_start_child_processes()
{
    int  core_count, k, pid;

    core_count = tan_get_core_count();

    if (tan_is_shipper())
        ++core_count;

    tan_vector_init(cpid);

    for (k = 0; k < core_count; ++k) {

        pid = fork();

        switch (pid) {

        case 0:

            if (tan_is_shipper() &&
                k == core_count - 1)
            {
                tan_set_process_name(TAN_SHIPPER_PROCESS);
                tan_shipper_process_loop();
            }

            tan_setaffinity(k);

            tan_set_process_name(TAN_WORKER_PROCESS);
            tan_worker_process_loop();

        case -1:

            tan_stderr_error(errno, "fork() failed");
            return TAN_ERROR;

        default:

            tan_vector_push_back(cpid, pid_t, pid);
        }
    }

    tan_set_process_name(TAN_MASTER_PROCESS);
    return TAN_OK;
}


static void
tan_worker_process_loop()
{
    if (tan_worker_process_init() != TAN_OK) {

        if (reap)
            exit(-1);

        tan_release_resources();
        kill(-ppid, SIGKILL);
    }

    tan_event_loop(server);

    tan_worker_process_exit();
}


static tan_int_t
tan_worker_process_init()
{
    tan_int_t  ret;

    tan_time_update();

    ret = tan_event_timer_init();
    if (ret != TAN_OK)
        return ret;

    ret = tan_idle_connections_init();
    if (ret != TAN_OK)
        return ret;

    if (tan_get_pg_cfg()->size) {

        if (tan_pgconn_init())
            return TAN_ERROR;
    }

    ret = tan_create_listen_socket(&server,
          tan_get_server_cfg()->port);

    if (ret != TAN_OK)
        return ret;

    ret = tan_epoll_init();
    if (ret != TAN_OK)
        return ret;

    ret = tan_add_listen_socket_to_epoll(server);
    if (ret != TAN_OK)
        return ret;

    return tan_load_user_api();
}


static void
tan_worker_process_exit()
{
    tan_unload_user_api();

    if (tan_get_pg_cfg()->size)
        tan_pgconn_free();

    exit(0);
}


static void
tan_notify_all_child_processes(int signo)
{
    int  k;

    for (k = 0; k < cpid.size; ++k) {

        if (kill(cpid.vec[k], signo)) {
            tan_log_warn(errno, "kill(%d, %d) failed",
                         cpid.vec[k], signo);
        }
    }
}


static int
tan_reap_zombie_process()
{
    int  zpid;

    zpid = waitpid(-1, NULL, WNOHANG);
    if (zpid > 0) {

        reap = 1;
        tan_log_info("child process %d exited abnormally", zpid);
    }

    return zpid;
}


static void
tan_refork_child_process(int zpid)
{
    int       pid;
    sigset_t  sigset;

    if (zpid <= 0)
        return;

    sigemptyset(&sigset);

    pid = fork();

    switch (pid) {

    case 0:

        if (sigprocmask(SIG_SETMASK, &sigset, NULL))
            tan_log_warn(errno, "sigprocmask(SIG_SETMASK) failed", NULL);

        if (tan_is_shipper() &&
            zpid == cpid.vec[cpid.size - 1])
        {
            tan_set_process_name(TAN_SHIPPER_PROCESS);
            tan_shipper_process_loop();

        } else {
            tan_setaffinity(tan_get_zpid_index(zpid));

            tan_set_process_name(TAN_WORKER_PROCESS);
            tan_worker_process_loop();
        }

    case -1:

        tan_log_crit(errno, "fork() failed", NULL);
        return;

    default:

        cpid.vec[tan_get_zpid_index(zpid)] = pid;
        return;
    }
}


static int
tan_get_zpid_index(int zpid)
{
    int  index;

    for (index = 0; index < cpid.size; ++index) {

        if (cpid.vec[index] == zpid)
            break;
    }

    return index;
}


static void
tan_release_resources()
{
    tan_remove_pidfile();
    tan_status_shm_free();
}

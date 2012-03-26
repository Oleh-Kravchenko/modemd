#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <getopt.h>
#include <sys/select.h>

#include "thread.h"
#include "queue.h"
#include "mc7700.h"

/*------------------------------------------------------------------------*/

const char help[] =
	"Usage: %s [-h] [-s SOCKET] [-p PID] [-l FILE] [-e] [-d]\n"
	"-h - show this help\n"
	"-s - file socket path (default: /var/run/%s.ctl)\n"
	"-p - pid file path (default: /var/run/%s.pid)\n"
	"-l - log to file\n"
	"-e - log to syslog\n"
	"-d - daemonize\n";

/*------------------------------------------------------------------------*/

static char srv_sock_path[256];
static char srv_pid_path[256];
static char srv_basename[256];

static char srv_filelog[256] = {0};
static int srv_syslog = 0;
static int srv_daemonize = 0;
static int srv_terminate = 0;
static int sock = -1;

/*------------------------------------------------------------------------*/

int analyze_parameters(int argc, char** argv)
{
    int param;

    /* receiving default parameters */
    strncpy(srv_basename, basename(argv[0]), sizeof(srv_basename) - 1);
    snprintf(srv_sock_path, sizeof(srv_sock_path), "/var/run/%s.ctl", srv_basename);
    snprintf(srv_pid_path, sizeof(srv_pid_path), "/var/run/%s.pid", srv_basename);

    /* analyze command line */
    while((param = getopt(argc, argv, "hs:p:l:ed")) != -1)
    {
        switch(param)
        {
            case 'h':
                printf(help, srv_basename, srv_basename, srv_basename);
                return(1);
            case 's':
                strncpy(srv_sock_path, optarg, sizeof(srv_sock_path) - 1);
                break;
            case 'p':
                strncpy(srv_pid_path, optarg, sizeof(srv_pid_path) - 1);
                break;
            case 'l':
                strncpy(srv_filelog, optarg, sizeof(srv_filelog) - 1);
                break;
            case 'e':
                srv_syslog = 1;
                break;
            case 'd':
                srv_daemonize = 1;
                break;
            default: /* '?' */
                printf(help, srv_basename, srv_basename, srv_basename);
                return(-1);
        }
    }

    return(0);
}

/*------------------------------------------------------------------------*/

int proccess_connection(int sock)
{
    cellulard_thread_t *priv_data;
    pthread_attr_t attr;
    pthread_t thread;
    int res = 0;

    /* allocate memory for private data */
    if(!(priv_data = (cellulard_thread_t*)malloc(sizeof(*priv_data))))
    {
        res = -1;
        goto err_malloc;
    }

    memset(priv_data, 0, sizeof(*priv_data));
    priv_data->sock = sock;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    /* creating thread */
    res = pthread_create(&thread, &attr, ThreadWrapper, priv_data);

    pthread_attr_destroy(&attr);

    if(res)
        free(priv_data);

err_malloc:
    return(res);
}

/*------------------------------------------------------------------------*/

int srv_run(void)
{
    struct sockaddr_un sa_bind;
    struct sockaddr_un sa_client;
    socklen_t sa_client_len;
    int sock_client = -1;
	struct timeval tv = {5, 0};
    int res = 0;
    fd_set rfds;

    /* creating socket server */
    if((sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        res = -1;
        goto err_socket;
    }

    /* filling address */
    memset(&sa_bind, 0, sizeof(sa_bind));
    sa_bind.sun_family = AF_LOCAL;
    strncpy(sa_bind.sun_path, srv_sock_path, sizeof(sa_bind.sun_path) - 1);

    if(bind(sock, (struct sockaddr *)&sa_bind, sizeof(sa_bind)))
    {
        res = -1;
        goto err_bind;
    }

    if(listen(sock, 1))
    {
        res = -1;
        goto err_listen;
    }

    sa_client_len = sizeof(sa_client);

    /* processing connections */
    while(!srv_terminate)
    {
#if 0
		FD_ZERO(&rfds);
		FD_SET(sock, &rfds);

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		if(!(res = select(sock + 1, &rfds, NULL, NULL, &tv)))
		{
			printf("Wait more %d\n", srv_terminate);
			continue;
		}

		if(!FD_ISSET(sock, &rfds))
		{
			printf("sig Wait more %d\n", srv_terminate);
			continue;
		}
#endif
		sock_client = accept(sock, (struct sockaddr*)&sa_client, &sa_client_len);

        /* create thread for connection */
        if((res = proccess_connection(sock_client)))
        {
            perror(NULL);

            close(sock_client);
        }

        /* and wait another connection */
        sa_client_len = sizeof(sa_client);
    }

err_listen:
    unlink(srv_sock_path);

err_bind:
    close(sock);

err_socket:
    return(res);
}

/*------------------------------------------------------------------------*/

void on_sigterm(int prm)
{
	printf("SIGTERM %d\n", prm);
	srv_terminate = 1;
	close(sock);
}

/*------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    int res;

    if((res = analyze_parameters(argc, argv)))
        return(res);

    /* show configuration */
    printf(
        "   Basename: %s\n"
        "Socket file: %s\n"
        "   PID file: %s\n"
        "   File log: %s\n"
        "     Syslog: %s\n"
        "  Daemonize: %s\n\n",
        srv_basename,
        srv_sock_path,
        srv_pid_path,
        *srv_filelog ? srv_filelog : "No",
        srv_syslog ? "Yes" : "No",
        srv_daemonize ? "Yes" : "No"
    );

    signal(SIGTERM, on_sigterm);
    signal(SIGINT, on_sigterm);

    /* run socket server */
    if(srv_run())
        perror(srv_basename);

    return 0;
}

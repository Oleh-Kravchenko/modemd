#include <stdio.h>
#include <stdlib.h>
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
#include <sys/select.h>
#include <string.h>
#include <syslog.h>

#include "conf.h"
#include "thread.h"

/*------------------------------------------------------------------------*/

int terminate = 0;
int sock = -1;

/*------------------------------------------------------------------------*/

int proccess_connection(int sock)
{
	modemd_client_thread_t *priv_data;
	pthread_attr_t attr;
	pthread_t thread;
	int res = 0;

	/* allocate memory for private data */
	if(!(priv_data = (modemd_client_thread_t*)malloc(sizeof(*priv_data))))
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
	int res = 0;
#if 0
	struct timeval tv = {5, 0};
	fd_set rfds;
#endif

	/* creating socket server */
	if((sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
	{
		res = -1;
		goto err_socket;
	}

	/* filling address */
	memset(&sa_bind, 0, sizeof(sa_bind));
	sa_bind.sun_family = AF_LOCAL;
	strncpy(sa_bind.sun_path, conf.sock_path, sizeof(sa_bind.sun_path) - 1);
	sa_bind.sun_path[sizeof(sa_bind.sun_path) - 1] = 0;

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
	while(!terminate)
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

#if 0
		terminate = 1;
		sleep(240);
#endif
	}

err_listen:
	unlink(conf.sock_path);

err_bind:
	close(sock);

err_socket:
	return(res);
}

/*------------------------------------------------------------------------*/

void on_sigterm(int prm)
{
	printf("SIGTERM %d\n", prm);
	terminate = 1;
	close(sock);
}

/*------------------------------------------------------------------------*/

void create_pid_file(const char* path)
{
	FILE *f = fopen(path, "w");

	if(f)
	{
		fprintf(f, "%d\n", getpid());
		fclose(f);
	}
}

/*------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
	modem_t* modem = NULL;
	int res;

	if((res = conf_read_cmdline(argc, argv)))
		return(res);

	/* show configuration */
	printf(
		"   Basename: %s\n"
		"Socket file: %s\n"
		"   PID file: %s\n"
		"     Syslog: %s\n\n",
		conf.basename,
		conf.sock_path,
		conf.pid_path,
		conf.syslog ? "Yes" : "No"
	);

	signal(SIGTERM, on_sigterm);
	signal(SIGINT, on_sigterm);

	if(*conf.pid_path)
		create_pid_file(conf.pid_path);

	if(conf.syslog)
		openlog(argv[0], LOG_PID, LOG_DAEMON);

	if(!stat(conf.sock_path, &(struct stat) {0}))
	{
		printf("(WW) socket file %s is exist. Removing..\n", conf.sock_path);
		unlink(conf.sock_path);
	}

	modem_init(NULL);

	if(*conf.port)
		modem = modem_open_by_port(conf.port);

	/* run socket server */
	if(srv_run())
		perror(conf.basename);

	modem_close(modem);

	modem_cleanup();

	if(conf.syslog)
		closelog();

	return 0;
}

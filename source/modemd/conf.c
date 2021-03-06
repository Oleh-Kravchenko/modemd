#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

#include "conf.h"

/*------------------------------------------------------------------------*/

modemd_conf_t conf;

/*------------------------------------------------------------------------*/

const char help[] =
	"Usage: %s [-h] [-s SOCKET] [-p PID] [-l] [-i BUS-DEV]\n"
	"-h - show this help\n"
	"-s - file socket path (default: /var/run/%s.ctl)\n"
	"-p - pid file path (default: /var/run/%s.pid)\n"
	"-l - log to syslog\n"
	"-i - initialize modem on port, for example 1-1\n";

/*------------------------------------------------------------------------*/

int conf_read_cmdline(int argc, char** argv)
{
	int param;

	/* setup default parameters */
	strncpy(conf.basename, basename(argv[0]), sizeof(conf.basename) - 1);
	conf.basename[sizeof(conf.basename) - 1] = 0;
	snprintf(conf.sock_path, sizeof(conf.sock_path), "/var/run/%s.ctl", conf.basename);
	snprintf(conf.pid_path, sizeof(conf.pid_path), "/var/run/%s.pid", conf.basename);
	*conf.port = 0;

	/* analyze command line */
	while((param = getopt(argc, argv, "hs:p:li:")) != -1)
	{
		switch(param)
		{
			case 'h':
				printf(help, conf.basename, conf.basename, conf.basename);
				return(1);

			case 's':
				strncpy(conf.sock_path, optarg, sizeof(conf.sock_path) - 1);
				conf.sock_path[sizeof(conf.sock_path) - 1] = 0;
				break;

			case 'p':
				strncpy(conf.pid_path, optarg, sizeof(conf.pid_path) - 1);
				conf.pid_path[sizeof(conf.pid_path) - 1] = 0;
				break;

			case 'i':
				strncpy(conf.port, optarg, sizeof(conf.port) - 1);
				conf.port[sizeof(conf.port) - 1] = 0;
				break;

			case 'l':
				conf.syslog = 1;
				break;

			default: /* '?' */
				printf(help, conf.basename, conf.basename, conf.basename);
				return(-1);
		}
	}

	return(0);
}

#ifndef __CONF_H
#define __CONF_H

typedef struct
{
	char sock_path[0x100];

	char pid_path[0x100];

	char basename[0x100];

	int syslog;

	int daemonize;
} modemd_conf_t;

/*------------------------------------------------------------------------*/

int conf_read_cmdline(int argc, char** argv);

/*------------------------------------------------------------------------*/

extern modemd_conf_t conf;

#endif /* __CONF_H */

#ifndef __CONF_H
#define __CONF_H

typedef struct
{
    char sock_path[256];

    char pid_path[256];

    char basename[256];

    int syslog;

    int daemonize;

    int verbose;
} modemd_conf_t;

/*------------------------------------------------------------------------*/

int conf_read_cmdline(int argc, char** argv);

/*------------------------------------------------------------------------*/

extern modemd_conf_t conf;

#endif /* __CONF_H */

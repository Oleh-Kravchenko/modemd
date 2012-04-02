#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#include "modem/modem.h"
#include "modem/modem_str.h"

/*------------------------------------------------------------------------*/

/* default name for daemon */
#define MODEMD_NAME "modemd"

/*------------------------------------------------------------------------*/

const char help[] =
    "Usage: %s [-h] [-s SOCKET] [-p PORT]\n"
    "-h - show this help\n"
    "-s - file socket path (default: /var/run/%s.ctl)\n"
    "-p - modem port, for example 1-1\n";

/*------------------------------------------------------------------------*/

static char srv_sock_path[0x100];
static char srv_modem_port[0x100];

/*------------------------------------------------------------------------*/

int analyze_parameters(int argc, char** argv)
{
    int param;

    /* receiving default parameters */
    snprintf(srv_sock_path, sizeof(srv_sock_path), "/var/run/%s.ctl", MODEMD_NAME);
    *srv_modem_port = 0;

    /* analyze command line */
    while((param = getopt(argc, argv, "hs:p:")) != -1)
    {
        switch(param)
        {
            case 's':
                strncpy(srv_sock_path, optarg, sizeof(srv_sock_path) - 1);
                srv_sock_path[sizeof(srv_sock_path) - 1] = 0;
                break;
            case 'p':
                strncpy(srv_modem_port, optarg, sizeof(srv_modem_port) - 1);
                srv_modem_port[sizeof(srv_modem_port) - 1] = 0;
                break;
            default: /* '?' */
                printf(help, argv[0], MODEMD_NAME);
                return(1);
        }
    }

    return(0);
}

/*------------------------------------------------------------------------*/

void modem_info(const char* port)
{
    modem_signal_quality_t sq;
    const struct tm* tm;
    char msg[0x100];
    modem_t* modem;
    time_t t;

    /* try open modem */
    if(!(modem = modem_open_by_port(port)))
        return;

    /* show modem info */
    printf("IMIE: [%s]\n", modem_get_imei(modem, msg, sizeof(msg)));

    printf("IMSI: [%s]\n", modem_get_imsi(modem, msg, sizeof(msg)));

    printf("Operator: [%s]\n", modem_get_operator_name(modem, msg, sizeof(msg)));

    printf("Network: [%s]\n", modem_get_network_type(modem, msg, sizeof(msg)));

    if(!modem_get_signal_quality(modem, &sq))
        printf("Signal: %d dBm, %d Level\n", sq.dbm, sq.level);

    if((t = modem_get_network_time(modem)))
    {
        tm = gmtime(&t);
        strftime(msg, sizeof(msg), "%Y.%m.%d %H:%M:%S", tm);
        printf("Modem time: %s", asctime(tm));
    }

    printf("Registration: %s\n", str_network_registration(modem_network_registration(modem)));

    /* close modem */
    modem_close(modem);
}

int main(int argc, char** argv)
{
    modem_info_t *mi;
    int res;

    if((res = analyze_parameters(argc, argv)))
        goto exit;

    /* show configuration */
    printf(
        "   Basename: %s\n"
        "Socket file: %s\n\n",
        argv[0], srv_sock_path
    );

    /* initialize modem library */
    if(modem_init(srv_sock_path))
    {
        perror(*argv);
        res = errno;
        goto exit;
    }

    if(*srv_modem_port)
    {
        /* show modem info by port and exit */
        modem_info(srv_modem_port);
        goto exit;
    }

    /* let's find some modem's */
    mi = modem_find_first();

    while(mi)
    {
        printf("%s %04hx:%04hx %s %s\n", mi->port, mi->id_vendor, mi->id_product, mi->manufacturer, mi->product);

        modem_info(mi->port);

        free(mi);

        mi = modem_find_next();
    }

    modem_cleanup();

exit:
    return(res);
}

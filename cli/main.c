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
	"-l - show all modems\n"
    "-p - modem port, for example 1-1\n";

/*------------------------------------------------------------------------*/

static char opt_sock_path[0x100];
static char opt_modem_port[0x100];
static int opt_show_modems = 0;

/*------------------------------------------------------------------------*/

int analyze_parameters(int argc, char** argv)
{
    int param;

    /* receiving default parameters */
    snprintf(opt_sock_path, sizeof(opt_sock_path), "/var/run/%s.ctl", MODEMD_NAME);
    *opt_modem_port = 0;

    /* analyze command line */
    while((param = getopt(argc, argv, "hs:p:l")) != -1)
    {
        switch(param)
        {
            case 's':
                strncpy(opt_sock_path, optarg, sizeof(opt_sock_path) - 1);
                opt_sock_path[sizeof(opt_sock_path) - 1] = 0;
                break;

            case 'p':
                strncpy(opt_modem_port, optarg, sizeof(opt_modem_port) - 1);
                opt_modem_port[sizeof(opt_modem_port) - 1] = 0;
                break;

            case 'l':
                opt_show_modems = 1;
                break;

            default: /* '?' */
                printf(help, argv[0], MODEMD_NAME);
                return(1);
        }
    }

    return(0);
}

/*------------------------------------------------------------------------*/

void print_modem_info(const char* port)
{
    modem_fw_version_t fw_info;
    modem_signal_quality_t sq;
    modem_oper_t *opers = NULL;
    const struct tm* tm;
    modem_info_t mi;
    char msg[0x100];
    modem_t* modem;
    time_t t;
    int i ;

    /* try open modem */
    if(!(modem = modem_open_by_port(port)))
        return;

    if(modem_get_info(modem, &mi))
        printf("Device: [port: %s] [%04hx:%04hx] [%s %s]\n", mi.port, mi.id_vendor, mi.id_product, mi.manufacturer, mi.product);

    /* show modem info */
    printf("IMEI: [%s]\n", modem_get_imei(modem, msg, sizeof(msg)));

    printf("IMSI: [%s]\n", modem_get_imsi(modem, msg, sizeof(msg)));

    printf("Operator: [%s]\n", modem_get_operator_name(modem, msg, sizeof(msg)));

    if(modem_get_network_type(modem, msg, sizeof(msg)))
        printf("Network: [%s]\n", msg);

    if(!modem_get_signal_quality(modem, &sq))
        printf("Signal: %d dBm, %d Level\n", sq.dbm, sq.level);

    if((t = modem_get_network_time(modem)))
    {
        tm = gmtime(&t);
        strftime(msg, sizeof(msg), "%Y.%m.%d %H:%M:%S", tm);
        printf("Modem time: %s", asctime(tm));
    }

    printf("Registration: %s\n", str_network_registration(modem_network_registration(modem)));

    if(modem_get_fw_version(modem, &fw_info))
    {
        tm = gmtime(&fw_info.release);
        strftime(msg, sizeof(msg), "%Y.%m.%d %H:%M:%S", tm);
        printf("Firmware: %s, Release: %s\n", fw_info.firmware, msg);
    }

    puts("Performing operator scan:");

    i = modem_operator_scan(modem, &opers);

    while(i > 0)
    {
        -- i;

        printf("stat=%d\nlong=%s\nshort=%s\nnumeric=%s\nact=%d\n\n",
            opers[i].stat, opers[i].longname, opers[i].shortname, opers[i].numeric, opers[i].act);
    }

    free(opers);

    /* close modem */
    modem_close(modem);
}

/*------------------------------------------------------------------------*/

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
        argv[0], opt_sock_path
    );

    /* initialize modem library */
    if(modem_init(opt_sock_path))
    {
        perror(*argv);
        res = errno;
        goto exit;
    }

    if(*opt_modem_port)
    {
        /* show modem info by port and exit */
        print_modem_info(opt_modem_port);
        goto exit;
    }

    /* let's find some modem's */
    mi = modem_find_first();

    while(mi)
    {
        print_modem_info(mi->port);

        free(mi);

        mi = modem_find_next();
    }

    modem_cleanup();

exit:
    return(res);
}

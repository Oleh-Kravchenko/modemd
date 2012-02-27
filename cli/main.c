#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "modem/modem.h"
#include "../lib/rpc.h"

/*-------------------------------------------------------------------------*/

#define CELLULARD_NAME "modemd"

const char help[] =
	"Usage: %s [-h] [-s SOCKET]\n"
	"-h - show this help\n"
	"-s - file socket path (default: /var/run/%s.ctl)\n";

/*-------------------------------------------------------------------------*/

char srv_sock_path[256];

/*-------------------------------------------------------------------------*/

int analyze_parameters(int argc, char** argv)
{
    int param;

    /* receiving default parameters */
    snprintf(srv_sock_path, sizeof(srv_sock_path), "/var/run/%s.ctl", CELLULARD_NAME);

    /* analyze command line */
    while((param = getopt(argc, argv, "hs:p:l:ed")) != -1)
    {
        switch(param)
        {
            case 'h':
                printf(help, argv[0], CELLULARD_NAME);
                return(1);
            case 's':
                strncpy(srv_sock_path, optarg, sizeof(srv_sock_path) - 1);
                break;
            default: /* '?' */
                printf(help, argv[0], CELLULARD_NAME);
                return(-1);
        }
    }

    return(0);
}

/*-------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    int res;

    if((res = analyze_parameters(argc, argv)))
        return(res);

    /* show configuration */
    printf(
        "   Basename: %s\n"
        "Socket file: %s\n\n",
        argv[0],
        srv_sock_path
    );

    if(modem_init(srv_sock_path) == 0)
    {
        modem_info_t *modem;

        modem = modem_find_first();

        while(modem)
        {
            printf("%s %04hx:%04hx %s %s\n", modem->port, modem->id_vendor, modem->id_product, modem->manufacturer, modem->product);

            free(modem);

            modem = modem_find_next();
        }

        modem_cleanup();
    }
    else
        perror(*argv);

    return 0;
}

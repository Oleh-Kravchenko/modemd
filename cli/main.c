#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>

#include "modem/modem.h"

/*------------------------------------------------------------------------*/

#define CELLULARD_NAME "modemd"

const char help[] =
	"Usage: %s [-h] [-s SOCKET]\n"
	"-h - show this help\n"
	"-s - file socket path (default: /var/run/%s.ctl)\n";

/*------------------------------------------------------------------------*/

char srv_sock_path[256];

/*------------------------------------------------------------------------*/

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

/*------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
    int res;
	const struct tm* tm;
	time_t t;
	char dt[0x100];
	char operator[0x100];

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
//        modem_info_t *mi;
        modem_t* modem;
        char imei[0x100];

//        mi = modem_find_first();

//        while(mi)
//        {
//            printf("%s %04hx:%04hx %s %s\n", mi->port, mi->id_vendor, mi->id_product, mi->manufacturer, mi->product);

            if((modem = modem_open_by_port("1-1")))
            {
                if(modem_get_imei(modem, imei, sizeof(imei)))
                    printf("IMIE: [%s]\n", imei);

				printf("Signal: %d dBm\n", modem_get_signal_quality(modem));
				
				t = modem_get_network_time(modem);
				tm = gmtime(&t);
				strftime(dt, sizeof(dt), "%Y.%m.%d %H:%M:%S", tm);
				printf("Time: %s\n", asctime(tm));

				printf("Operator: %s\n", modem_get_operator_name(modem, operator, sizeof(operator)));
				
				modem_close(modem);
//            }

//            free(mi);

//            mi = modem_find_next();
        }

        modem_cleanup();
    }
    else
        perror(*argv);

    return 0;
}

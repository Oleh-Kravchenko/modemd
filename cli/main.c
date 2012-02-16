#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define CELLULARD_NAME "cellulard"

/*-------------------------------------------------------------------------*/

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









    struct sockaddr_un sa_bind;
    char buf[0xffff];
    int buf_recv;
    int sock = -1;

    /* creating socket client */
    if((sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        res = -1;
        goto err_socket;
    }

    /* filling address */
    memset(&sa_bind, 0, sizeof(sa_bind));
    sa_bind.sun_family = AF_LOCAL;
    strncpy(sa_bind.sun_path, srv_sock_path, sizeof(sa_bind.sun_path) - 1);

    if(connect(sock, (struct sockaddr*)&sa_bind, sizeof(sa_bind)))
    {
        perror(argv[0]);
        goto err_connect;
    }

    while((buf_recv = recv(sock, buf, sizeof(buf), 0)) > 0)
    {
        puts(buf);

        break;
    }

err_connect:
    close(sock);

err_socket:
    return 0;
}

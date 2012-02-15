#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdint.h>

//#include "modem.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>


/*-------------------------------------------------------------------------*/

const char help[] =
	"Usage: %s [-h] -c SOCKET_PATH\n"
	"-c - connect to cellulard socket\n"
	"-h - show this help\n";

/*-------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
    int param, i;
    char sock_path[256];

    printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

    *sock_path = 0;

    while((param = getopt(argc, argv, "c:h")) != -1)
    {
        switch(param)
        {
            case 'c':
                printf("Connecting to: %s\n", optarg);
                strncpy(sock_path, optarg, sizeof(sock_path) - 1);
                break;
            case 'h':
                printf(help, argv[0]);
                return(0);
            default: /* '?' */
                printf(help, argv[0]);
                return(1);
        }
    }

    printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

    if(!*sock_path)
    {
        printf(help, argv[0]);
        return(0);
    }

    printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

    struct sockaddr_un sunaddr;
    int sock = -1;
    uint8_t buf[0xffff];
    ssize_t buf_recv;


    printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

    if((sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        puts("socket() failed");
        return(0);
    }

    printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

    memset(&sunaddr, 0, sizeof(sunaddr));

    sunaddr.sun_family = AF_LOCAL;
    strncpy(sunaddr.sun_path, sock_path, sizeof(sunaddr.sun_path) - 1);

    printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

    if(!connect(sock, (struct sockaddr*)&sunaddr, sizeof(sunaddr)))
    {
        while(1)
        {
            buf_recv = recv(sock, buf, sizeof(buf), 0);

            if(buf_recv <= 0)
                continue;

            buf[buf_recv] = 0;
            printf("%s", buf);
            for(i = 0; i < buf_recv; ++ i)
                putchar(buf[i]);
        }
    }
    else
    {
        puts("connect() failed");
    }

    printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

    close(sock);


    return(0);
}

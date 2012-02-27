#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "modem/types.h"
#include "rpc.h"
 
/*-------------------------------------------------------------------------*/

static int sock = -1;

static struct sockaddr_un sa_bind;

/*-------------------------------------------------------------------------*/

int modem_init(const char* socket_path)
{
    int res = 0;

    /* creating socket client */
    if((sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        res = -1;
        goto err_socket;
    }

    /* filling address */
    memset(&sa_bind, 0, sizeof(sa_bind));
    sa_bind.sun_family = AF_LOCAL;
    strncpy(sa_bind.sun_path, socket_path, sizeof(sa_bind.sun_path) - 1);

    /* connecting */
    if(connect(sock, (struct sockaddr*)&sa_bind, sizeof(sa_bind)))
    {
        close(sock);

        sock = -1;
        res = -1;
    }

err_socket:
    return(res);
}

/*-------------------------------------------------------------------------*/

void modem_cleanup(void)
{
    if(sock > 0)
    {
        close(sock);
        sock = -1;
    }
}

/*-------------------------------------------------------------------------*/

modem_info_t* modem_find_first(void)
{
    packet_t *p;
    modem_info_t* res = NULL;

    p = malloc(sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.data_len = 0;
    p->hdr.func_len = strlen(__func__);
    p->func = malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->data = NULL;

    rpc_send(sock, p);
    rpc_free(p);

    p = rpc_recv(sock);
    rpc_print(p);

    if(p->data)
    {
        res = malloc(sizeof(*res));
        memcpy(res, p->data, sizeof(*res));
    }

    rpc_free(p);

    return(res);
}

/*-------------------------------------------------------------------------*/

modem_info_t* modem_find_next(void)
{
    packet_t *p;
    modem_info_t* res = NULL;

    p = malloc(sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.data_len = 0;
    p->hdr.func_len = strlen(__func__);
    p->func = malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->data = NULL;

    rpc_send(sock, p);
    rpc_free(p);

    p = rpc_recv(sock);
    rpc_print(p);

    if(p->data)
    {
        res = malloc(sizeof(*res));
        memcpy(res, p->data, sizeof(*res));
    }

    rpc_free(p);

    return(res);
}

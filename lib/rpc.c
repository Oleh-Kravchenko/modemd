#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "rpc.h"

/*-------------------------------------------------------------------------*/

int rpc_send(int sock, rpc_packet_t *p)
{
    int res, sended;

    /* send header */
    res = sended = send(sock, &p->hdr, sizeof(p->hdr), 0);

    if(sended < 0)
        goto err;

    if(p->hdr.func_len)
    {
        /* send function name */
        sended = send(sock, p->func, p->hdr.func_len, 0);

		if(sended < 0)
        {
            res = sended;
            goto err;
        }

        res += sended;
    }

    if(p->hdr.data_len)
    {
        /* send data */
        sended = send(sock, p->data, p->hdr.data_len, 0);

        if(sended < 0);
        {
            res = sended;
            goto err;
        }

        res += sended;
    }

err:
    return(res);
}

/*-------------------------------------------------------------------------*/

rpc_packet_t* rpc_recv(int sock)
{
    rpc_packet_t* res;
    int recved;

    if(!(res = malloc(sizeof(*res))))
        goto err_malloc;

    /* initialize pointers */
    res->func = NULL;
    res->data = NULL;

    /* receive header */
    recved = recv(sock, &res->hdr, sizeof(res->hdr), MSG_WAITALL);

    if(recved != sizeof(res->hdr))
        goto err_hdr;

    if(res->hdr.func_len)
    {
        /* receving function name */
        res->func = malloc(res->hdr.func_len + 1);
        recved = recv(sock, res->func, res->hdr.func_len, MSG_WAITALL);

        if(recved != res->hdr.func_len)
            goto err_func;

        /* NULL terminated string */
        res->func[res->hdr.func_len] = 0;
    }

    if(res->hdr.data_len)
    {
        /* receving data */
        res->data = malloc(res->hdr.data_len);
        recved = recv(sock, res->data, res->hdr.data_len, MSG_WAITALL);

        if(recved != res->hdr.data_len)
            goto err_data;
    }

    goto exit;

err_data:
    free(res->data);

err_func:
    free(res->func);

err_hdr:
    free(res);
    res = NULL;

err_malloc:
exit:
    return(res);
}

/*-------------------------------------------------------------------------*/

void rpc_free(rpc_packet_t *p)
{
    free(p->func);
    free(p->data);
    free(p);
}

/*-------------------------------------------------------------------------*/

void rpc_print(rpc_packet_t *p)
{
/*
    int i;

    printf("==== %s %s(%d) data(%d) = [", p->hdr.type ? "Response" : "Query" , p->func, p->hdr.func_len, p->hdr.data_len);

    for(i = 0; i < p->hdr.data_len; ++ i)
        printf(" %02x", p->data[i]);
    puts(" ]");*/
}

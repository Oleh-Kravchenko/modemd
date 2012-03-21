#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "rpc.h"

/*------------------------------------------------------------------------*/

rpc_packet_t* rpc_create(rpc_packet_type_t type, const char* func, uint8_t* data, uint16_t data_len)
{
    rpc_packet_t* res;

    if(!(res = (rpc_packet_t*)malloc(sizeof(*res))))
        goto err;

    /* filling header */
    res->hdr.type = type;
    res->hdr.func_len = strlen(func);
    res->hdr.data_len = data_len;

    if(!(res->func = malloc(res->hdr.func_len + 1)))
        goto err_func;

    /* function name */
    strncpy(res->func, func, res->hdr.func_len + 1);

    if(data_len) /* data is required field */
    {
        if(!(res->data = malloc(data_len)))
            goto err_data;
        
        memcpy(res->data, data, data_len);
    }
    else
        res->data = NULL;
    
    goto exit;

err_data:
    free(res->func);

err_func:
    free(res);
    res = NULL;

err:
exit:
    return(res);
}

/*------------------------------------------------------------------------*/

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

/*------------------------------------------------------------------------*/

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

/*------------------------------------------------------------------------*/

void rpc_free(rpc_packet_t *p)
{
    if(!p)
        return;

    free(p->func);
    free(p->data);
    free(p);
}

/*------------------------------------------------------------------------*/

void rpc_print(rpc_packet_t *p)
{
//#ifdef _DEBUG
    int i;

    printf("==== %s %s(%d) data(%d) = [", p->hdr.type ? "Response" : "Query" , p->func, p->hdr.func_len, p->hdr.data_len);

    for(i = 0; i < p->hdr.data_len; ++ i)
        printf(" %02x", p->data[i]);
    puts(" ]");
//#endif /* _DEBUG */
}

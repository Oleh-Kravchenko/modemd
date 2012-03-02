#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "modem/types.h"
#include "rpc.h"
 
/*------------------------------------------------------------------------*/

static int sock = -1;

static struct sockaddr_un sa_bind;

/*------------------------------------------------------------------------*/

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

/*------------------------------------------------------------------------*/

void modem_cleanup(void)
{
    if(sock > 0)
    {
        close(sock);
        sock = -1;
    }
}

/*------------------------------------------------------------------------*/

#define RPC_FUNCTION_VOID(result, funcname)                                \
                                                                           \
    result funcname##_res_unpack(rpc_packet_t*);                           \
                                                                           \
    result funcname(void)                                                  \
    {                                                                      \
        rpc_packet_t* p;                                                   \
        result res;                                                        \
                                                                           \
        /* build packet and send it */                                     \
        p = (rpc_packet_t*)malloc(sizeof(*p));                             \
        memset((void*)p, 0, sizeof(*p));                                   \
        p->hdr.type = TYPE_QUERY;                                          \
        p->hdr.func_len = strlen(#funcname);                               \
        p->func = (char*)malloc(p->hdr.func_len);                          \
        memcpy(p->func, #funcname, p->hdr.func_len);                       \
        rpc_send(sock, p);                                                 \
        rpc_free(p);                                                       \
                                                                           \
        /* receive result and unpack it */                                 \
        p = rpc_recv(sock);                                                \
        res = funcname##_res_unpack(p);                                    \
        rpc_free(p);                                                       \
                                                                           \
        /* returning result */                                             \
        return(res);                                                       \
    }

/*------------------------------------------------------------------------*/

#define RPC_FUNCTION(result, funcname, ...)                                \
                                                                           \
    result funcname##_res_unpack(rpc_packet_t*, __VA_ARGS__);              \
                                                                           \
    result funcname(__VA_ARGS__)                                           \
    {                                                                      \
        rpc_packet_t* p;                                                   \
        result res;                                                        \
                                                                           \
        /* build packet and send it */                                     \
        p = (rpc_packet_t*)malloc(sizeof(*p));                             \
        memset((void*)p, 0, sizeof(*p));                                   \
        p->hdr.type = TYPE_QUERY;                                          \
        p->hdr.func_len = strlen(#funcname);                               \
        p->func = (char*)malloc(p->hdr.func_len);                          \
        memcpy(p->func, #funcname, p->hdr.func_len);                       \
        funcname##_prm_unpack(p, __VA_ARGS__);                             \
        rpc_send(sock, p);                                                 \
        rpc_free(p);                                                       \
                                                                           \
        /* receive result and unpack it */                                 \
        p = rpc_recv(sock);                                                \
        res = funcname##_res_unpack(p, __VA_ARGS__);                       \
        rpc_free(p);                                                       \
                                                                           \
        /* returning result */                                             \
        return(res);                                                       \
    }

/*------------------------------------------------------------------------*/

RPC_FUNCTION_VOID(modem_info_t*, modem_find_first)
RPC_FUNCTION_VOID(modem_info_t*, modem_find_next)

/*------------------------------------------------------------------------*/

modem_info_t* modem_find_first_res_unpack(rpc_packet_t* p)
{
    modem_info_t* res = NULL;

    if(p->data && p->hdr.data_len == sizeof(*res))
    {
        res = malloc(sizeof(*res));
        memcpy(res, p->data, sizeof(*res));
    }
}

/*------------------------------------------------------------------------*/

modem_info_t* modem_find_next_res_unpack(rpc_packet_t* p)
    /* the result of modem_find_next_res_unpack() have the same format with
    function modem_find_first_res_unpack(), so we can use it as a alias */
    __attribute__((alias("modem_find_first_res_unpack")));

/*------------------------------------------------------------------------*/

modem_t* modem_open_by_port(const char* port)
{
    return((modem_t*)1);
}

/*------------------------------------------------------------------------*/

void modem_close(modem_t* modem)
{
}

/*------------------------------------------------------------------------*/

char* modem_get_imei(modem_t* modem, char* imei, int len)
{
    strncpy(imei, "012626000027332", len);
    
    return(imei);
}

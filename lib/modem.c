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

/*------------------------------------------------------------------------*/

int modem_init(const char* socket_path)
{
    struct sockaddr_un sa_srv;
    int res = 0;

    /* creating socket client */
    if((sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
    {
        res = -1;
        goto err_socket;
    }

    /* filling address */
    memset(&sa_srv, 0, sizeof(sa_srv));
    sa_srv.sun_family = AF_LOCAL;
    strncpy(sa_srv.sun_path, socket_path, sizeof(sa_srv.sun_path) - 1);

    /* connecting */
    if(connect(sock, (struct sockaddr*)&sa_srv, sizeof(sa_srv)))
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
    if(sock != -1)
    {
        /* if socket valid close them */
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
                                                                           \
        res = funcname##_res_unpack(p);                                    \
                                                                           \
        rpc_free(p);                                                       \
                                                                           \
        /* returning result */                                             \
        return(res);                                                       \
    }

/*------------------------------------------------------------------------*/

RPC_FUNCTION_VOID(modem_info_t*, modem_find_first)
RPC_FUNCTION_VOID(modem_info_t*, modem_find_next)
RPC_FUNCTION_VOID(modem_network_reg_t, modem_network_registration)

/*------------------------------------------------------------------------*/

modem_info_t* modem_find_first_res_unpack(rpc_packet_t* p)
{
    modem_info_t* res = NULL;

    if(p && p->hdr.data_len == sizeof(*res))
    {
        res = malloc(sizeof(*res));
        memcpy(res, p->data, sizeof(*res));
    }

    return(res);
}

/*------------------------------------------------------------------------*/

modem_info_t* modem_find_next_res_unpack(rpc_packet_t* p)
    /* the result of modem_find_next_res_unpack() have the same format with
    function modem_find_first_res_unpack(), so we can use it as a alias */
    __attribute__((alias("modem_find_first_res_unpack")));

/*------------------------------------------------------------------------*/

modem_network_reg_t modem_network_registration_res_unpack(rpc_packet_t* p)
{
    modem_network_reg_t res = MODEM_NETWORK_REG_UNKNOWN;

    if(p && p->hdr.data_len == sizeof(res))
        res = *((modem_network_reg_t*)p->data);

    return(res);
}

/*------------------------------------------------------------------------*/

modem_t* modem_open_by_port(const char* port)
{
    rpc_packet_t* p;

    /* build packet and send it */
    p = (rpc_packet_t*)malloc(sizeof(*p));
    memset((void*)p, 0, sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.func_len = strlen(__func__);
    p->func = (char*)malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->hdr.data_len = strlen(port);
    p->data = malloc(p->hdr.data_len);
    memcpy(p->data, port, p->hdr.data_len);
    rpc_send(sock, p);
    rpc_free(p);

    /* receive result and unpack it */
    p = rpc_recv(sock);
//    res = modem_open_by_port_res_unpack(p);
    rpc_free(p);

    /* returning result */
    return((modem_t*)p->data);
}

/*------------------------------------------------------------------------*/

void modem_close(modem_t* modem)
{
    rpc_packet_t* p;

    /* build packet and send it */
    p = (rpc_packet_t*)malloc(sizeof(*p));
    memset((void*)p, 0, sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.func_len = strlen(__func__);
    p->func = (char*)malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->hdr.data_len = 0;
    rpc_send(sock, p);
    rpc_free(p);

    /* receive result and unpack it */
    p = rpc_recv(sock);
    rpc_free(p);
}

/*------------------------------------------------------------------------*/

char* modem_get_imei(modem_t* modem, char* imei, int len)
{
    rpc_packet_t* p;

    /* build packet and send it */
    p = (rpc_packet_t*)malloc(sizeof(*p));
    memset((void*)p, 0, sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.func_len = strlen(__func__);
    p->func = (char*)malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->hdr.data_len = 0;
    p->data = NULL;
    rpc_send(sock, p);
    rpc_free(p);

    /* receive result and unpack it */
    p = rpc_recv(sock);

    if(p)
    {
        len = (p->hdr.data_len > len ? len : p->hdr.data_len);
        strncpy(imei, (const char*)p->data, len);
        imei[len] = 0;

        rpc_free(p);

        return(imei);
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

int modem_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq)
{
    rpc_packet_t* p;
    int res = -1;

    /* build packet and send it */
    p = (rpc_packet_t*)malloc(sizeof(*p));
    memset((void*)p, 0, sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.func_len = strlen(__func__);
    p->func = (char*)malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->hdr.data_len = 0;
    p->data = NULL;
    rpc_send(sock, p);
    rpc_free(p);

    /* receive result and unpack it */
    p = rpc_recv(sock);

    if(p && p->hdr.data_len == sizeof(*sq))
    {
        memcpy(sq, p->data, sizeof(*sq));
        res = 0;
    }

    rpc_free(p);

    return(res);
}

/*------------------------------------------------------------------------*/

time_t modem_get_network_time(modem_t* modem)
{
    rpc_packet_t* p;
    time_t res = 0;

    /* build packet and send it */
    p = (rpc_packet_t*)malloc(sizeof(*p));
    memset((void*)p, 0, sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.func_len = strlen(__func__);
    p->func = (char*)malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->hdr.data_len = 0;
    p->data = NULL;
    rpc_send(sock, p);
    rpc_free(p);

    /* receive result and unpack it */
    p = rpc_recv(sock);

    if(p && p->hdr.data_len == sizeof(time_t))
        res = *((time_t*)p->data);

    rpc_free(p);

    return(res);
}

/*------------------------------------------------------------------------*/

char* modem_get_imsi(modem_t* modem,char *imsi, int len)
{
    rpc_packet_t* p;

    /* build packet and send it */
    p = (rpc_packet_t*)malloc(sizeof(*p));
    memset((void*)p, 0, sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.func_len = strlen(__func__);
    p->func = (char*)malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->hdr.data_len = 0;
    p->data = NULL;
    rpc_send(sock, p);
    rpc_free(p);

    /* receive result and unpack it */
    p = rpc_recv(sock);

    if(p)
    {
        len = (p->hdr.data_len > len ? len : p->hdr.data_len);
        strncpy(imsi, (const char*)p->data, len);
        imsi[len] = 0;

        rpc_free(p);

        return(imsi);
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

char* modem_get_operator_name(modem_t *modem, char *oper, int len)
{
    rpc_packet_t* p;

    /* build packet and send it */
    p = (rpc_packet_t*)malloc(sizeof(*p));
    memset((void*)p, 0, sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.func_len = strlen(__func__);
    p->func = (char*)malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->hdr.data_len = 0;
    p->data = NULL;
    rpc_send(sock, p);
    rpc_free(p);

    /* receive result and unpack it */
    p = rpc_recv(sock);

    if(p)
    {
        len = (p->hdr.data_len > len ? len : p->hdr.data_len);
        strncpy(oper, (const char*)p->data, len);
        oper[len] = 0;

        rpc_free(p);

        return(oper);
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

char* modem_get_network_type(modem_t* modem, char *network, int len)
{
    rpc_packet_t* p;

    /* build packet and send it */
    p = (rpc_packet_t*)malloc(sizeof(*p));
    memset((void*)p, 0, sizeof(*p));
    p->hdr.type = TYPE_QUERY;
    p->hdr.func_len = strlen(__func__);
    p->func = (char*)malloc(p->hdr.func_len);
    memcpy(p->func, __func__, p->hdr.func_len);
    p->hdr.data_len = 0;
    p->data = NULL;
    rpc_send(sock, p);
    rpc_free(p);

    /* receive result and unpack it */
    p = rpc_recv(sock);

    if(p)
    {
        len = (p->hdr.data_len > len ? len : p->hdr.data_len);
        strncpy(network, (const char*)p->data, len);
        network[len] = 0;

        rpc_free(p);

        return(network);
    }

    return(NULL);
}

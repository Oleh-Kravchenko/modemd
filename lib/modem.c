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
        p = funcname##_prm_pack(__VA_ARGS__);                              \
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

/*RPC_FUNCTION(modem_t*, modem_open_by_port, const char* port)*/

/*------------------------------------------------------------------------*/

modem_info_t* modem_find_first_res_unpack(rpc_packet_t* p)
{
    modem_info_t* res = NULL;

    if(p->data && p->hdr.data_len == sizeof(*res))
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

modem_t* modem_open_by_port(const char* port)
{
	rpc_packet_t* p;
	modem_t* res = NULL;

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
//	res = modem_open_by_port_res_unpack(p);
	rpc_free(p);

	/* returning result */
	return((modem_t*)p->data);
#if 0
	return((modem_t*)1);
#endif
}

/*------------------------------------------------------------------------*/

void modem_close(modem_t* modem)
{
	rpc_packet_t* p;
	modem_t* res = NULL;

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
//	res = modem_open_by_port_res_unpack(p);
    strncpy(imei, (const char*)p->data, (p->hdr.data_len > len ? len : p->hdr.data_len) - 1);

	rpc_free(p);

    return(imei);
}

int modem_get_channel_activity(modem_t* modem,int *failure)
{
//	*failure = eRegistrationStatus;
//    return eRegistrationFailure == eERR_NO_ERROR;

	*failure = 2; //eERR_NO_RESPONSE;
	return 0; //fail
}

int modem_set_new_params(modem_t* modem)
{
// CLteSierra::InitiateParametersReset()
	return 1; //success
}

int modem_start_available_networks_scan(modem_t* modem)
{
	// CLteSierra::StartAvailableNetworksScan()
	return 1; //success
}

int modem_networks_scan_is_running(modem_t* modem)
{
	// return CCellCore::bNetworksScanIsRunning
	return 0;
}

int modem_get_registration_status(modem_t* modem)
{
	//return CCellCore::eRegistrationStatus
	return 1; //eREGISTERED_HOME
}

int modem_get_signal_quality(modem_t* modem,int *dBm)
{
	//return CCellCore::GetSignalQuality( int *dBm )
	*dBm = -1;
	return 0; //CSQ_NOT_KNOWN
}

int modem_change_pin(modem_t* modem,char* old_pin,char* new_pin)
{
	//CCellCore::ChangePIN()
	 return 0; //OK
}

char* modem_get_imsi(modem_t* modem,char *imsi,int len)
{
	//pszIMSIString
    strncpy(imsi, "255071040257705", len);
    return(imsi);
}

char* modem_get_operator_name(modem_t* modem,char *operator,int len)
{
	//CCellEngine::GetOperatorName()
	strncpy(operator, "Kyivstar", len);
    return(operator);
}

time_t modem_get_network_time_date(modem_t* modem,int wait_answer)
{
	//CLteSierra::GetNetworkTimeDate
   return 0;
}

char* modem_get_network_type(modem_t* modem,char *network_type,int len)
{
	//CLteSierra::GetNetworkType()
	strncpy(network_type, "LTE", len);
    return(network_type);
}

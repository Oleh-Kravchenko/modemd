#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>

#include "modem/types.h"
#include "rpc.h"
 
/*------------------------------------------------------------------------*/

#define __DEFAULT_TRIES 3

/*------------------------------------------------------------------------*/

static int sock = -1;

/*------------------------------------------------------------------------*/

int modem_init(const char* socket_path)
{
	struct sockaddr_un sa_srv;

	if(sock != -1)
		/* already initialized */
		return(0);

	/* creating socket client */
	if((sock = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)
		return(-1);

	/* filling address */
	memset(&sa_srv, 0, sizeof(sa_srv));
	sa_srv.sun_family = AF_LOCAL;
	strncpy(sa_srv.sun_path, socket_path, sizeof(sa_srv.sun_path) - 1);
	sa_srv.sun_path[sizeof(sa_srv.sun_path) - 1] = 0;

	/* connecting */
	if(connect(sock, (struct sockaddr*)&sa_srv, sizeof(sa_srv)))
	{
		close(sock);

		sock = -1;

		return(-1);
	}

	return(0);
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

#define RPC_FUNCTION_PRM_VOID(result, funcname)							\
																		   \
	result funcname##_res_unpack(rpc_packet_t*);						   \
																		   \
	result funcname(void)												  \
	{																	  \
		rpc_packet_t* p;												   \
		result res;														\
																		   \
		/* build packet and send it */									 \
		p = rpc_create(TYPE_QUERY, __func__, NULL, 0);					 \
		rpc_send(sock, p);												 \
		rpc_free(p);													   \
																		   \
		/* receive result and unpack it */								 \
		p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);				\
																		   \
		res = funcname##_res_unpack(p);									\
																		   \
		rpc_free(p);													   \
																		   \
		/* returning result */											 \
		return(res);													   \
	}

/*------------------------------------------------------------------------*/

#define RPC_FUNCTION_RES_STR(funcname)									 \
																		   \
	char* funcname(modem_t *modem, char *s, int len)					   \
	{																	  \
		rpc_packet_t* p;												   \
		char* res = NULL;												  \
																		   \
		/* build packet and send it */									 \
		p = rpc_create(TYPE_QUERY, __func__, NULL, 0);					 \
		rpc_send(sock, p);												 \
		rpc_free(p);													   \
																		   \
		/* receive result and unpack it */								 \
		p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);				\
																		   \
		if(p && p->hdr.data_len)										   \
		{																  \
			len = (p->hdr.data_len >= len ? len - 1 : p->hdr.data_len);	\
			memcpy(s, (const char*)p->data, len);						  \
			s[len] = 0;													\
																		   \
			res = s;													   \
		}																  \
																		   \
		rpc_free(p);													   \
																		   \
		return(res);													   \
	}

/*------------------------------------------------------------------------*/

RPC_FUNCTION_PRM_VOID(modem_network_reg_t, modem_network_registration)

/*------------------------------------------------------------------------*/

RPC_FUNCTION_RES_STR(modem_get_imei)
RPC_FUNCTION_RES_STR(modem_get_imsi)
RPC_FUNCTION_RES_STR(modem_get_operator_name)
RPC_FUNCTION_RES_STR(modem_get_network_type)

/*------------------------------------------------------------------------*/

usb_device_info_t* modem_find_first_res_unpack(rpc_packet_t* p)
{
	usb_device_info_t* res = NULL;

	if(p && p->hdr.data_len == sizeof(*res))
	{
		res = malloc(sizeof(*res));
		memcpy(res, p->data, sizeof(*res));
	}

	return(res);
}

/*------------------------------------------------------------------------*/

usb_device_info_t* modem_find_next_res_unpack(rpc_packet_t* p)
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

modem_find_t* modem_find_first(usb_device_info_t* mi)
{
	modem_find_first_next_t res;
	rpc_packet_t* p;

	memset(&res, 0, sizeof(res));

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p->data && p->hdr.data_len == sizeof(res))
	{
		memcpy(&res, p->data, sizeof(res));
		memcpy(mi, &res.mi, sizeof(*mi));
	}

	rpc_free(p);

	/* returning result */
	/* on this stage this result is not necessary */
	return(res.find);
}

/*------------------------------------------------------------------------*/

modem_find_t* modem_find_next(modem_find_t* find, usb_device_info_t* mi)
{
	modem_find_first_next_t res;
	rpc_packet_t* p;

	memset(&res, 0, sizeof(res));

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, (uint8_t*)&find, sizeof(find));
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p->data && p->hdr.data_len == sizeof(res))
	{
		memcpy(&res, p->data, sizeof(res));
		memcpy(mi, &res.mi, sizeof(*mi));
	}

	rpc_free(p);

	/* returning result */
	/* on this stage this result is not necessary */
	return(res.find);
}

/*------------------------------------------------------------------------*/

modem_t* modem_open_by_port(const char* port)
{
	rpc_packet_t* p;
	modem_t* res;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, (uint8_t*)port, strlen(port));
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	res = (modem_t*)p->data;

	rpc_free(p);

	/* returning result */
	/* on this stage this result is not necessary */
	return(res);
}

/*------------------------------------------------------------------------*/

void modem_close(modem_t* modem)
{
	rpc_packet_t* p;

	if(!modem)
		return;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);
	rpc_free(p);
}

/*------------------------------------------------------------------------*/

void modem_conf_reload(modem_t* modem)
{
	rpc_packet_t* p;

	if(!modem)
		return;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);
	rpc_free(p);
}

/*------------------------------------------------------------------------*/

int modem_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq)
{
	rpc_packet_t* p;
	int res = -1;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

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
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(time_t))
		res = *((time_t*)p->data);

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_change_pin(modem_t* modem, const char* old_pin, const char* new_pin)
{
	modem_change_pin_t pc;
	rpc_packet_t* p;
	int res = -1;

	strncpy(pc.old_pin, old_pin, sizeof(pc.old_pin));
	strncpy(pc.new_pin, new_pin, sizeof(pc.new_pin));

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, (uint8_t*)&pc, sizeof(pc));
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len)
		res = 0;

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

modem_fw_ver_t* modem_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info)
{
	modem_fw_ver_t* res = NULL;
	rpc_packet_t* p;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(*fw_info))
	{
		memcpy(fw_info, (const char*)p->data, sizeof(*fw_info));

		res = fw_info;
	}

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

usb_device_info_t* modem_get_info(modem_t* modem, usb_device_info_t *mi)
{
	usb_device_info_t* res = NULL;
	rpc_packet_t* p;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(*mi))
	{
		memcpy(mi, (const char*)p->data, sizeof(*mi));

		res = mi;
	}

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_get_cell_id(modem_t* modem)
{
	rpc_packet_t* p;
	int res = 0;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(int32_t))
		res = *((int32_t*)p->data);

	rpc_free(p);

	return res;
}

/*------------------------------------------------------------------------*/

int modem_operator_scan(modem_t* modem, modem_oper_t** opers)
{
	rpc_packet_t* p;
	int res = 0;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && (p->hdr.data_len % sizeof(modem_oper_t) == 0))
	{
		if((*opers = malloc(p->hdr.data_len)))
		{
			memcpy(*opers, p->data, p->hdr.data_len);

			/* calculate number of operator items */
			res = p->hdr.data_len / sizeof(modem_oper_t);
		}
	}

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

char* modem_at_command(modem_t* modem, const char* query)
{
	rpc_packet_t* p;
	char* res = NULL;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, (uint8_t*)query, strlen(query) + 1);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len)
		if((res = malloc(p->hdr.data_len + 1)))
		{
			memcpy(res, p->data, p->hdr.data_len);
			res[p->hdr.data_len] = 0;
		}

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_operator_scan_start(modem_t* modem, const char* file)
{
	rpc_packet_t* p;
	int res = -1;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, (uint8_t*)file, strlen(file) + 1);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len)
		res = 0;

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_operator_scan_is_running(modem_t* modem)
{
	rpc_packet_t* p;
	int res = -1;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(int8_t))
		res = *((int8_t*)p->data);

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_get_last_error(modem_t* modem)
{
	rpc_packet_t* p;
	int res = -1;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(int32_t))
		res = *((int32_t*)p->data);

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_set_wwan_profile(modem_t* modem, modem_data_profile_t* profile)
{
	rpc_packet_t* p;
	int res = -1;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, (uint8_t*)profile, sizeof(*profile));
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len)
		res = 0;

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_start_wwan(modem_t* modem)
{
	rpc_packet_t* p;
	int res = -1;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(int32_t))
		res = *((int32_t*)p->data);

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_stop_wwan(modem_t* modem)
{
	rpc_packet_t* p;
	int res = -1;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(int32_t))
		res = *((int32_t*)p->data);

	rpc_free(p);

	return(res);
}

/*------------------------------------------------------------------------*/

modem_state_wwan_t modem_state_wwan(modem_t* modem)
{
	modem_state_wwan_t res = MODEM_STATE_WWAN_UKNOWN;
	rpc_packet_t* p;

	/* build packet and send it */
	p = rpc_create(TYPE_QUERY, __func__, NULL, 0);
	rpc_send(sock, p);
	rpc_free(p);

	/* receive result and unpack it */
	p = rpc_recv_func(sock, __func__, __DEFAULT_TRIES);

	if(p && p->hdr.data_len == sizeof(res))
		res = *((modem_state_wwan_t*)p->data);

	rpc_free(p);

	return(res);
}

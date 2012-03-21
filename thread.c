#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
       #include <sys/types.h>
       #include <sys/stat.h>
       #include <fcntl.h>

#include "thread.h"
#include "modem/types.h"
#include "lib/rpc.h"
#include "lib/modem_int.h"
#include "lib/utils.h"
#include "mc7700.h"


/*------------------------------------------------------------------------*/

typedef rpc_packet_t* (*rpc_function_t)(cellulard_thread_t*, rpc_packet_t*);

/*------------------------------------------------------------------------*/

typedef struct
{
    char name[0xff];

    rpc_function_t func;
} rpc_function_info_t;

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_find_first_packet(cellulard_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    modem_info_t *mi;
    int mi_len = sizeof(*mi);

    if((mi = modem_find_first(&priv->dir)))
        printf("%s %04hx:%04hx %s %s\n", mi->port, mi->id_vendor, mi->id_product, mi->manufacturer, mi->product);
	else
		mi_len = 0;

    res = rpc_create(TYPE_RESPONSE, "modem_find_first", (uint8_t*)mi, mi_len);

    free(mi);
    
    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_find_next_packet(cellulard_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    modem_info_t *mi;
    int mi_len = sizeof(*mi);

    if((mi = modem_find_next(&priv->dir)))
        printf("%s %04hx:%04hx %s %s\n", mi->port, mi->id_vendor, mi->id_product, mi->manufacturer, mi->product);
	else
		mi_len = 0;

    res = rpc_create(TYPE_RESPONSE, "modem_find_next", (uint8_t*)mi, mi_len);

    free(mi);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_open_by_port(cellulard_thread_t* priv, rpc_packet_t* p)
{
	char path[0xff] = {"A"};
	char tty[0xff];
	int path_len, modem;
    rpc_packet_t *res = NULL;

	path_len = p->hdr.data_len > sizeof(path) ? sizeof(path) : p->hdr.data_len;
	memcpy(path, p->data, path_len);
	path[path_len] = 0;

	printf("==== %s\n", path);

	modem_get_at_port_name(path, tty, sizeof(tty));
	printf("==== %s\n", tty);

	modem = mc7700_open(tty);

    res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&modem, sizeof(modem));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_close(cellulard_thread_t* priv, rpc_packet_t* p)
{
	mc7700_destroy();
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_imei(cellulard_thread_t* priv, rpc_packet_t* p)
{
	printf("==== %s:%d %s()\n", __FILE__, __LINE__, __func__);
#define _AT_CGSN "AT+CGSN\r\n"
	queue_add(thread_priv.q, _AT_CGSN, strlen(_AT_CGSN));
	printf("==== %s:%d %s()\n", __FILE__, __LINE__, __func__);

    return(rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)"12345678", 8));
}

/*------------------------------------------------------------------------*/

const rpc_function_info_t rpc_functions[] = {
    {"modem_find_first", modem_find_first_packet},
    {"modem_find_next", modem_find_next_packet},
    {"modem_open_by_port", modem_open_by_port},
    {"modem_close", modem_close},
    {"modem_get_imei", modem_get_imei},
    {},
};

/*------------------------------------------------------------------------*/

void* ThreadWrapper(void* prm)
{
    cellulard_thread_t* priv = prm;
    modem_info_t *mi;
    rpc_packet_t *p_in = NULL, *p_out = NULL;

    while((p_in = rpc_recv(priv->sock)))
    {
        rpc_print(p_in);

        if(p_in->hdr.type != TYPE_QUERY)
        {
            rpc_free(p_in);
            continue;
        }

        const rpc_function_info_t *rpc_func = rpc_functions;

        while(rpc_func->func)
        {
			if(strcmp(rpc_func->name, p_in->func) != 0)
			{
				++ rpc_func;

				continue;
			}

			p_out = rpc_func->func(priv, p_in);
			
			if(p_out)
			{
				rpc_send(priv->sock, p_out);
				rpc_print(p_out);
				rpc_free(p_out);
			}
	
			break;
		}

		rpc_free(p_in);
    }

    /* cleanup resources */
    close(priv->sock);

    if(priv->dir)
        closedir(priv->dir);

    free(priv);

    return(NULL);
}

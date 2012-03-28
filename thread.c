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
#include <time.h>

#include "thread.h"
#include "mc7700.h"
#include "modem/types.h"
#include "lib/rpc.h"
#include "lib/modem_int.h"
#include "lib/utils.h"
#include "hardware.h"

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
	char tty[0xff] = {0};
	int path_len, modem = 0;
    rpc_packet_t *res = NULL;
    int modem_wakeup_tries;

	path_len = p->hdr.data_len > sizeof(path) ? sizeof(path) : p->hdr.data_len;
	memcpy(path, p->data, path_len);
	path[path_len] = 0;

    modem_wakeup_tries = 3;

	while(!modem_get_at_port_name(path, tty, sizeof(tty)) && modem_wakeup_tries)
	{
        init_port(path);
        printf("(II) Wait modem wakeup on port %s\n", path);
        sleep(10);
        -- modem_wakeup_tries;
	}

    if(*tty)
    {
    	printf("==== %s -> %s\n", path, tty);

    	modem = mc7700_open(tty);
    }

    res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&modem, sizeof(modem));

	return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_close(cellulard_thread_t* priv, rpc_packet_t* p)
{
	mc7700_destroy();

	return(rpc_create(TYPE_RESPONSE, __func__, NULL, 0));
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_imei(cellulard_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	mc7700_query_t *q;

	q = mc7700_query_create("AT+CGSN\r\n", "\r\n([0-9]+)\r\n\r\nOK\r\n");
	mc7700_query_execute(thread_priv.q, q);

	/* cutting IMEI number from the reply */
	if(q->n_subs)
		res = rpc_create(
			TYPE_RESPONSE, __func__,
			(uint8_t*)q->answer + q->re_subs[1].rm_so,
			q->re_subs[1].rm_eo - q->re_subs[1].rm_so
		);

	mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_signal_quality(cellulard_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	mc7700_query_t *q;
	char rssi[16], ber[16];
	int16_t signal;

	q = mc7700_query_create("AT+CSQ\r\n", "\r\n\\+CSQ: ([0-9]+),([0-9]+)\r\n\r\nOK\r\n");
	mc7700_query_execute(thread_priv.q, q);

	/* cutting IMEI number from the reply */
	if(q->n_subs)
	{
		memset(rssi, 0, sizeof(rssi));
		memset(ber, 0, sizeof(ber));

		memcpy(rssi, q->answer + q->re_subs[1].rm_so, q->re_subs[1].rm_eo - q->re_subs[1].rm_so);
		memcpy(ber, q->answer + q->re_subs[2].rm_so, q->re_subs[2].rm_eo - q->re_subs[2].rm_so);

		printf("rssi: %s ber: %s\n", rssi, ber);
		signal = -113 + atoi(rssi) * 2;

		res = rpc_create(
			TYPE_RESPONSE, __func__,
			(uint8_t*)&signal,
			sizeof(signal)
		);
	}

	mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_network_time(cellulard_thread_t* priv, rpc_packet_t* p)
{
	rpc_packet_t *res = NULL;
	mc7700_query_t *q;
	char dt[32];
	struct tm tm;
	time_t t;

 	q = mc7700_query_create("AT!TIME?\r\n", "!TIME:.*\r\n([0-9,/]+\r\n[0-9,:]+) \\(local\\)\r\n[0-9,/]+\r\n[0-9,:]+ \\(UTC\\)\r\n\r\n\r\nOK\r\n");
	mc7700_query_execute(thread_priv.q, q);

	/* cutting TIME from the reply */
	if(q->re_subs)
	{
		memset(dt, 0, sizeof(dt));
//		memset(time, 0, sizeof(time));

		memcpy(dt, q->answer + q->re_subs[1].rm_so, q->re_subs[1].rm_eo - q->re_subs[1].rm_so);
//		memcpy(time, q->answer + q->re_subs[2].rm_so, q->re_subs[2].rm_eo - q->re_subs[2].rm_so);

		memset(&tm, 0, sizeof(tm));
		
		/* parsing date and time */
		//sscanf(dt,"%d/%d/%d\r\n%d:%d:%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
		char *strptime(const char *s, const char *format, struct tm *tm);

		strptime(dt, "%Y/%m/%d\r\n%H:%M:%S", &tm);

		t = mktime(&tm);
		
		printf("unix time stamp: %d\n", (int)t);
		
		res = rpc_create(
			TYPE_RESPONSE, __func__,
			(uint8_t*)&t,
			sizeof(t)
		);
	}

	mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

const rpc_function_info_t rpc_functions[] = {
    {"modem_find_first", modem_find_first_packet},
    {"modem_find_next", modem_find_next_packet},
    {"modem_open_by_port", modem_open_by_port},
    {"modem_close", modem_close},
    {"modem_get_imei", modem_get_imei},
	{"modem_get_signal_quality", modem_get_signal_quality},
	{"modem_get_network_time", modem_get_network_time},
    {},
};

/*------------------------------------------------------------------------*/

void* ThreadWrapper(void* prm)
{
    cellulard_thread_t* priv = prm;
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

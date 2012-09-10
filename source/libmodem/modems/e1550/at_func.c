#include <stdio.h>
#include <unistd.h>

#include <modem/types.h>

#include "proto.h"

#include "at/at_queue.h"
#include "at/at_common.h"

#include "utils/re.h"
#include "utils/str.h"

/*------------------------------------------------------------------------*/

char* e1550_at_get_network_type(modem_t* modem, char* network, size_t len)
{
	static const char* sys_submode_str[] = {
		"No service",
		"GSM",
		"GPRS",
		"EDGE",
		"WCDMA",
		"HSDPA",
		"HSUPA",
		"HSDPA+HSUPA",
		"TD_SCDMA",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+ (64QAM)",
		"HSPA+ (MIMO)",
	};

	int sys_submode = 0;
	at_queue_t* at_q;
	at_query_t* q;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(NULL);

	q = at_query_create("AT^SYSINFO\r\n", "\r\n\\^SYSINFO:[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]*,([0-9]+)\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
		sys_submode = re_atoi(q->result, q->pmatch + 1);

	at_query_free(q);

	if(!sys_submode)
		return(NULL);

	/* when the returned value of <sys_submode> is out of (0~18),
		you should treat it as 4 (WCDMA) by default */
	if(sys_submode < 0 || sys_submode > 18)
		sys_submode = 4;

	strncpy(network, sys_submode_str[sys_submode], len - 1);
	network[len - 1] = 0;

	return(network);
}

/*------------------------------------------------------------------------*/

int e1550_at_get_freq_bands(modem_t* modem, freq_band_t** band_list)
{
	at_queue_t* at_q;
	at_query_t* q;

	regmatch_t* pmatch;
	freq_band_t* item;
	unsigned int i, j = 0;
    size_t nmatch;
	char* bands = NULL;
	char* index;
	char* name;
	int nbands = 0;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(nbands);

	q = at_query_create("AT^SYSCFG=?\r\n", "\\^SYSCFG:\\([0-9,]+\\),\\([0-9\\-]+\\),\\((.*)\\),\\([0-9\\-]+\\),\\([0-9\\-]+\\)\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
		bands = re_strdup(q->result, q->pmatch + 1);

	at_query_free(q);

	if(!bands)
		return(nbands);

	while(!re_parse(bands + j, "\\(([0-9A-Fa-f]+),\"([A-Za-z0-9/ ]+)\"\\),?", &nmatch, &pmatch))
	{
		*band_list = item = realloc(*band_list, sizeof(freq_band_t) * (nbands + 1));

		index = re_strdup(bands + j, pmatch + 1);
		name = re_strdup(bands + j, pmatch + 2); trim(name);

		j += pmatch->rm_eo;

		/* band index */
		sscanf(index, "%02X", &i);
		item[nbands].index = i;

		/* band name */
		strncpy(item[nbands].name, name, sizeof(item[nbands].name) - 1);
		item[nbands].name[sizeof(item[nbands].name) - 1] = 0;

		free(index);
		free(name);
		free(pmatch);

		++ nbands;
	}

	free(bands);

	return(nbands);
}

/*------------------------------------------------------------------------*/

int e1550_at_get_freq_band(modem_t* modem)
{
	at_queue_t* at_q;
	at_query_t* q;

	regmatch_t* pmatch;
    size_t nmatch;
	int res = -1;
	char* band;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT^SYSCFG?\r\n", "\\^SYSCFG:[0-9]+,[0-9]+,([0-9A-F]+),[0-9]+,[0-9]+\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		band = re_strdup(q->result, q->pmatch + 1);
		sscanf(band, "%X", (unsigned int*)&res);
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

int e1550_at_set_freq_band(modem_t* modem, int band_index)
{
	char cmd[0x100];

	snprintf(cmd, sizeof(cmd), "AT^SYSCFG=16,3,%X,2,4\r\n", band_index);

	return(at_raw_ok(modem, cmd));
}

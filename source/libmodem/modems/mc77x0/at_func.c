#include <stdio.h>
#include <unistd.h>

#include <modem/types.h>

#include "proto.h"

#include "at/at_queue.h"
#include "at/at_common.h"

#include "utils/re.h"
#include "utils/str.h"

/*------------------------------------------------------------------------*/

int mc77x0_at_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq)
{
	at_queue_t* at_q;
	at_query_t* q;
	int res = -1, nrssi, nber;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	sq->dbm = 0;
	sq->level = 0;

	q = at_query_create("AT+CSQ\r\n", "([0-9]+), ?([0-9]+)\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		nrssi = re_atoi(q->result, q->pmatch + 1);
		nber = re_atoi(q->result, q->pmatch + 2);

		if(nrssi > 31)
			goto exit;

		/* calculation dBm */
		sq->dbm = nrssi * 2 - 113;

		/* calculation signal level */
		sq->level += !!(sq->dbm >= -109);
		sq->level += !!(sq->dbm >= -95);
		sq->level += !!(sq->dbm >= -85);
		sq->level += !!(sq->dbm >= -73);
		sq->level += !!(sq->dbm >= -65);

		res = 0;
	}

exit:
	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

modem_network_reg_t at_network_registration_cereg(modem_t* modem)
{
	modem_network_reg_t nr = MODEM_NETWORK_REG_UNKNOWN;
	at_queue_t* at_q;
 	at_query_t* q;
	int nnr;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(nr);

	q = at_query_create("AT+CEREG?\r\n", "\r\n\\+CEREG: [0-9],([0-9])\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		/* cutting registration status from the reply and check value */
		nnr = re_atoi(q->result, q->pmatch + 1);

		/* check value */
		if(nnr >= MODEM_NETWORK_REG_FAILED && nnr < MODEM_NETWORK_REG_COUNT)
			nr = nnr;
	}

	at_query_free(q);

	return(nr);
}

/*------------------------------------------------------------------------*/

modem_network_reg_t mc77x0_at_network_registration(modem_t* modem)
{
	modem_network_reg_t nr = MODEM_NETWORK_REG_UNKNOWN;
	char s[0x100], w[0x100], z[0x100];
	at_queue_t* at_q;
	at_query_t* q;

	/* workaround for buggy firmware */
	/* check +CREG, +CEREG and !GSTATUS */

	/* check standard AT+CREG */
	switch((nr = at_network_registration(modem)))
	{
		case MODEM_NETWORK_REG_HOME:
		case MODEM_NETWORK_REG_SEARCHING:
		case MODEM_NETWORK_REG_ROAMING:
		case MODEM_NETWORK_REG_DENIED:
			return(nr);

		case MODEM_NETWORK_REG_FAILED:
		case MODEM_NETWORK_REG_UNKNOWN:
		default:
			break;
	}

	/* check Sierra AT+CEREG */
	switch((nr = at_network_registration_cereg(modem)))
	{
		case MODEM_NETWORK_REG_HOME:
		case MODEM_NETWORK_REG_SEARCHING:
		case MODEM_NETWORK_REG_ROAMING:
		case MODEM_NETWORK_REG_DENIED:
			return(nr);

		case MODEM_NETWORK_REG_FAILED:
		case MODEM_NETWORK_REG_UNKNOWN:
		default:
			break;
	}

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(nr);

	/* check registration through AT!GSTATUS */
	q = at_query_create("AT!GSTATUS?\r\n", "\r\n\\!GSTATUS: \r\n.*\tPS state: *([A-Za-z]+) *\r\n.*\r\n(GMM \\(PS\\) state:|EMM state:) *([A-Za-z]+) *\t([A-Za-z]+ ?[A-Za-z]*) *\r\n.*\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

#if 0
	/* suppress errors if regular expression do is not match */
	queue->last_error = -1;
#endif

	if(!at_query_is_error(q))
	{
		re_strncpy(s, sizeof(s), q->result, q->pmatch + 1);
		re_strncpy(w, sizeof(w), q->result, q->pmatch + 3);
		re_strncpy(z, sizeof(z), q->result, q->pmatch + 4);

		printf("AT!GSTATUS: [%s] [%s] [%s]\n", s, w, z);

		if
		(
			strcasestr(s, "Attached") &&
			strcasestr(w, "REGISTERED") &&
			strcasestr(z, "NORMAL")
		)
			nr = MODEM_NETWORK_REG_HOME;
		else
			nr = MODEM_NETWORK_REG_SEARCHING;
	}

	at_query_free(q);

	return(nr);
}


/*------------------------------------------------------------------------*/

uint8_t mc77x0_at_get_freq_band(modem_t* modem)
{
	at_queue_t* at_q;
	at_query_t* q;
	char band[0x100];
	unsigned int res = -1;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT!BAND?\r\n", "\r\nIndex, Name\r\n([0-9A-Z]+), .+\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		re_strncpy(band, sizeof(band), q->result, q->pmatch + 1);
		sscanf(band, "%02X", &res);
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

int mc77x0_at_get_freq_bands(modem_t* modem, freq_band_t** band_list)
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

	q = at_query_create("AT!BAND=?\r\n", "\r\nIndex, Name\r\n(.*)\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
		bands = re_strdup(q->result, q->pmatch + 1);

	at_query_free(q);

	if(!bands)
		return(nbands);

	while(!re_parse(bands + j, "([0-9A-Fa-f]{2}), ([A-Za-z0-9 ]+)\r\n", &nmatch, &pmatch))
	{
		*band_list = item = realloc(*band_list, sizeof(freq_band_t) * (nbands + 1));

		index = re_strdup(bands + j, pmatch + 1);
		name = re_strdup(bands + j, pmatch + 2); trim(name);

		j += re_strlen(pmatch);

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

int mc77x0_at_set_freq_band(modem_t* modem, uint8_t band_index)
{
	char cmd[0x100];

	snprintf(cmd, sizeof(cmd), "AT!BAND=%02X\r\n", band_index);

	return(at_raw_ok(modem, cmd));
}

/*------------------------------------------------------------------------*/

char* at_get_network_type_gstatus(modem_t* modem, char *network, size_t len)
{
	at_queue_t* at_q;
	at_query_t* q;
	char *res = NULL;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT!GSTATUS?\r\n", "\r\n.*\r\nSystem mode: *([A-Za-z\\+]+) .*\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		re_strncpy(network, len, q->result, q->pmatch + 1);

		res = network;
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

void mc77x0_modem_sw_reset(modem_t* modem)
{
	void *thread_res;

	/* termination scan routine */
	if(modem->scan.thread)
		pthread_join(modem->scan.thread, &thread_res);

	/* reseting modem by command */
	at_raw_ok(modem, "AT!RESET");

	/* suspend queues */
	modem_queues_suspend(modem);

	sleep(10);

	/* resume queues */
	modem_queues_resume(modem);
}


/*------------------------------------------------------------------------*/

char* mc77x0_at_get_network_type(modem_t* modem, char* network, size_t len)
{
	at_queue_t* at_q;
	at_query_t* q;
	char* res = NULL;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT*CNTI=0\r\n", "\r\n\\*CNTI: 0,(.+)\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		re_strncpy(network, len, q->result, q->pmatch + 1);

		res = network;
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

char* mc7750_at_get_network_type(modem_t* modem, char *network, size_t len)
{
	if(mc77x0_at_get_network_type(modem, network, len))
		return(network);

	if(at_get_network_type_gstatus(modem, network, len))
		return(network);

	return(NULL);
}

/*------------------------------------------------------------------------*/

modem_fw_ver_t* mc77x0_at_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info)
{
	modem_fw_ver_t* res = NULL;
	char release[0x100];
	at_queue_t* at_q;
	at_query_t* q;
	struct tm tm;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT+CGMR\r\n", "\r\n.*(SWI.*) .* .* ([0-9,/]+ [0-9,:]+)\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	/* cutting firmware version from the answer */
	if(!at_query_is_error(q))
	{
		/* create result */
		re_strncpy(fw_info->firmware, sizeof(fw_info->firmware), q->result, q->pmatch + 1);
		re_strncpy(release, sizeof(release), q->result, q->pmatch + 2);

		/* parsing date and time */
		strptime(release, "%Y/%m/%d\r\n%H:%M:%S", &tm);
		fw_info->release = mktime(&tm);

		res = fw_info;
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

time_t mc77x0_at_get_network_time(modem_t* modem)
{
	at_queue_t* at_q;
	at_query_t* q;
	char dt[24];
	struct tm tm;
	time_t t = 0;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(t);

	q = at_query_create("AT!TIME?\r\n", "!TIME:.*\r\n([0-9,/]+\r\n[0-9,:]+) \\(local\\)\r\n[0-9,/]+\r\n[0-9,:]+ \\(UTC\\)\r\n\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	/* cutting TIME from the reply */
	if(!at_query_is_error(q))
	{
		re_strncpy(dt, sizeof(dt), q->result, q->pmatch + 1);

		/* parsing date and time */
		strptime(dt, "%Y/%m/%d\r\n%H:%M:%S", &tm);

		t = mktime(&tm);
	}

	at_query_free(q);

	return(t);
}

/*------------------------------------------------------------------------*/

int mc77x0_at_get_cell_id(modem_t* modem)
{
	int cell_id = 0;
	at_queue_t* at_q;
	at_query_t *q;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(cell_id);

	q = at_query_create("AT!GSMINFO?\r\n", "!GSMINFO:.*\r\nCell ID:[\t]*([0-9]+)\r\n.*\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	/* cutting Cell ID number from the reply */
	if(!at_query_is_error(q))
		cell_id = re_atoi(q->result, q->pmatch + 1);

	at_query_free(q);

	return(cell_id);
}

/*------------------------------------------------------------------------*/

char* mc77x0_at_get_ccid(modem_t* modem, char* s, size_t len)
{
	char ccid[21] = {0};
	at_queue_t* at_q;
	at_query_t* q;
	int i;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(NULL);

	q = at_query_create("AT+CRSM=176,12258,0,0,10\r\n", "\r\n\\+CRSM: 144,0,\"([0-9AFaf]+)\"\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	/* cutting ccid from the reply */
	if(!at_query_is_error(q))
	{
		re_strncpy(ccid, sizeof(ccid), q->result, q->pmatch + 1);

		for(i = 0; i < 20; i += 2)
		{
			if(ccid[i] == ccid[i + 1])
				continue;

			/* swapping symbols */
			ccid[i] = ccid[i] + ccid[i + 1];
			ccid[i + 1] = ccid[i] - ccid[i + 1];
			ccid[i] = ccid[i] - ccid[i + 1];
		}

		trim_r_esc(ccid, "F");
		trim_r_esc(ccid, "f");

		strncpy(s, ccid, len - 1);
		s[len - 1] = 0;
	}

	at_query_free(q);

	return(*ccid ? s : NULL);
}

/*------------------------------------------------------------------------*/

int mc77x0_at_start_wwan(modem_t* modem)
{
	return(at_raw_ok(modem, "AT!SCACT=1,3\r\n"));
}

/*------------------------------------------------------------------------*/

int mc77x0_at_stop_wwan(modem_t* modem)
{
	return(at_raw_ok(modem, "AT!SCACT=0\r\n"));
}

/*------------------------------------------------------------------------*/

int mc77x0_at_state_wwan(modem_t* modem)
{
	at_queue_t* at_q;
	at_query_t* q;

	regmatch_t* pmatch;
    size_t nmatch;
	char* s = NULL;
	size_t i = 0;
	int res = 0;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT!SCACT?\r\n", "\r\n(.*)\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
		s = re_strdup(q->result, q->pmatch + 1);

	at_query_free(q);

	if(!s)
		return(res);

	while(!re_parse(s + i, "!SCACT: [0-9]+,([0-9]+)\r\n", &nmatch, &pmatch))
	{
		if((res = re_atoi(s + i, pmatch + 1)))
			break;

		i += re_strlen(pmatch);

		free(pmatch);
	}

	free(s);

	return(res);
}

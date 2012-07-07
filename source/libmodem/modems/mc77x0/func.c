#include <stdio.h>
#include <unistd.h>

#include <modem/types.h>

#include "proto.h"
#include "at/at_queue.h"
#include "at/at_common.h"

#include "utils/re.h"

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

unsigned int mc77x0_at_get_freq_band(modem_t* modem)
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

char* at_get_network_type_gstatus(modem_t* modem, char *network, int len)
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

char* mc77x0_at_get_network_type(modem_t* modem, char* network, int len)
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

char* mc7750_at_get_network_type(modem_t* modem, char *network, int len)
{
	if(mc77x0_at_get_network_type(modem, network, len))
		return(network);

	if(at_get_network_type_gstatus(modem, network, len))
		return(network);

	return(NULL);
}

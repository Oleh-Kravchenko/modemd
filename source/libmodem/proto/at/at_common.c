#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <modem/types.h>

#include "utils/re.h"
#include "utils/str.h"
#include "queue.h"
#include "at_queue.h"
#include "at_query.h"
#include "at_common.h"
#include "at_utils.h"
#include "proto.h"

/*------------------------------------------------------------------------*/

modem_cpin_state_t at_cpin_state(modem_t* modem)
{
	modem_cpin_state_t res = MODEM_CPIN_STATE_UNKNOWN;
	at_queue_t* at_q;
	at_query_t* q;
	char cpin_st[8];

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT+CPIN?\r\n", "\r\n\\+CPIN: ((READY)|(SIM PIN)|(SIM PUK))\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		re_strncpy(cpin_st, sizeof(cpin_st), q->result, q->pmatch + 1);

		if(strcmp(cpin_st, "READY") == 0)
			res = MODEM_CPIN_STATE_READY;
		else if(strcmp(cpin_st, "SIM PIN") == 0)
			res = MODEM_CPIN_STATE_PIN;
		else if(strcmp(cpin_st, "SIM PUK") == 0)
			res = MODEM_CPIN_STATE_PUK;
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

int at_cpin_pin(modem_t* modem, const char* pin)
{
	at_queue_t* at_q;
	at_query_t* q;
	char cmd[32];
	int res = -1;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	snprintf(cmd, sizeof(cmd), "AT+CPIN=\"%s\"\r\n", pin);
	q = at_query_create(cmd, "\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	res = at_query_is_error(q);

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

int at_cpin_puk(modem_t* modem, const char* puk, const char* pin)
{
	at_queue_t* at_q;
	at_query_t* q;
	char cmd[0x100];
	int res = -1;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	snprintf(cmd, sizeof(cmd), "AT+CPIN=\"%s\",\"%s\"\r\n", puk, pin);
	q = at_query_create(cmd, "\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	res = at_query_is_error(q);

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

int at_raw_ok(queue_t* queue, const char* cmd)
{
	at_query_t* q;
	int res;

	q = at_query_create(cmd, "\r\nOK\r\n");
	q->timeout = 10;
	at_query_exec(queue, q);

	res = at_query_is_error(q);

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

char* at_get_imsi(modem_t* modem, char* imsi, size_t len)
{
	at_queue_t* at_q;
	at_query_t* q;
	char *res = NULL;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT+CIMI\r\n", "\r\n([0-9]+)\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	/* cutting IMSI number from the reply */
	if(!at_query_is_error(q))
	{
		re_strncpy(imsi, len, q->result, q->pmatch + 1);

		res = imsi;
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

char* at_get_imei(modem_t* modem, char* imei, size_t len)
{
	at_queue_t* at_q;
	at_query_t* q;
	char* res = NULL;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT+CGSN\r\n", "\r\n([0-9]+)\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		re_strncpy(imei, len, q->result, q->pmatch + 1);

		res = imei;
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

int at_operator_scan(modem_t* modem, modem_oper_t** opers)
{
	at_queue_t* at_q;
	at_query_t* q;
	int nopers = 0;
	char *sopers;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(nopers);

	q = at_query_create("AT+COPS=?\r\n", "\r\n\\+COPS: (.+),,\\(.+\\),\\(.+\\)\r\n\r\nOK\r\n");
	q->timeout = 120;

	at_query_exec(at_q->queue, q);

	if(at_query_is_error(q))
		goto exit;

	/* cutting operators from the answer */
	if(!(sopers = re_strdup(q->result, q->pmatch + 1)))
		goto err;

	/* parsing operator list */
	nopers = at_parse_cops_list(sopers, opers);

err:
	free(sopers);

exit:
	at_query_free(q);

	return(nopers);
}

/*------------------------------------------------------------------------*/

modem_network_reg_t at_network_registration(modem_t* modem)
{
	modem_network_reg_t nr = MODEM_NETWORK_REG_UNKNOWN;
	at_queue_t* at_q;
	at_query_t* q;
	int nnr;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(nr);

	q = at_query_create("AT+CREG?\r\n", "\r\n\\+CREG: [0-9],([0-9])\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		/* cutting registration status from the reply and check value */
		nnr = re_atoi(q->result, q->pmatch + 1);

		/* check value */
		if(nnr >= 0 && nnr <= 5)
			nr = nnr;
	}

	at_query_free(q);

	return(nr);
}

/*------------------------------------------------------------------------*/

modem_cops_mode_t at_cops_mode(modem_t* modem)
{
	modem_cops_mode_t nr = MODEM_COPS_MODE_UNKNOWN;
	at_queue_t* at_q;
	at_query_t* q;
	int nnr;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(nr);

	q = at_query_create("AT+COPS?\r\n", "\\+COPS: ([01234]),?.*\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		nnr = re_atoi(q->result, q->pmatch + 1);
		nr = (nnr > MODEM_COPS_MODE_UNKNOWN && nnr < MODEM_COPS_MODE_MAX)
			? nnr : MODEM_COPS_MODE_UNKNOWN;
	}

	at_query_free(q);

	return(nr);
}

/*------------------------------------------------------------------------*/

int at_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq)
{
	at_queue_t* at_q;
	at_query_t* q;
	int res = -1, nrssi, nber;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	sq->dbm = 0;
	sq->level = 0;

	q = at_query_create("AT+CSQ\r\n", "\r\n\\+CSQ: ([0-9]+),([0-9]+)\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	/* cutting IMEI number from the reply */
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

modem_fw_ver_t* at_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info)
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

	/* cutting Operator name from the answer */
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

char* at_get_network_type(modem_t* modem, char* network, int len)
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

char* at_get_operator_name(modem_t* modem, char* oper, int len)
{
	at_queue_t* at_q;
	at_query_t* q;
	int res_ok;
	char *res = NULL;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	/* setup format of +COPS as a string */
	q = at_query_create("AT+COPS=3,0\r\n", "\r\nOK\r\n");
	at_query_exec(at_q->queue, q);
	res_ok = !at_query_is_error(q);
	at_query_free(q);

	if(!res_ok)
		return(res);

	q = at_query_create("AT+COPS?\r\n", "\r\n\\+COPS: [0-9],[0-9],\"(.+)\",[0-9]\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	/* cutting Operator name from the answer */
	if(!at_query_is_error(q))
	{
		re_strncpy(oper, len, q->result, q->pmatch + 1);

		res = oper;
	}

	at_query_free(q);

	return(res);
}

/*-------------------------------------------------------------------------*/

char* at_get_operator_number(modem_t* modem, char* oper_number, int len)
{
	at_queue_t* at_q;
	at_query_t* q;
	int res_ok;
	char *res = NULL;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	/* setup format of +COPS as a string */
	q = at_query_create("AT+COPS=3,2\r\n", "\r\nOK\r\n");
	at_query_exec(at_q->queue, q);
	res_ok = !at_query_is_error(q);
	at_query_free(q);

	if(!res_ok)
		return(res);

	q = at_query_create("AT+COPS?\r\n", "\r\n\\+COPS: [0-9],[0-9],\"(.+)\",[0-9]\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	/* cutting Operator name from the answer */
	if(!at_query_is_error(q))
	{
		re_strncpy(oper_number, len, q->result, q->pmatch + 1);

		res = oper_number;
	}

	at_query_free(q);

	return(res);
}

/*-------------------------------------------------------------------------*/

time_t at_get_network_time(modem_t* modem)
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

/*-------------------------------------------------------------------------*/

int at_get_cell_id(modem_t* modem)
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

/*-------------------------------------------------------------------------*/

char* at_get_ccid(modem_t* modem, char* s, size_t len)
{
	char ccid[21] = {0};
	at_queue_t* at_q;
	at_query_t* q;
	int i;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(NULL);

	q = at_query_create("AT+CRSM=176,12258,0,0,10\r\n", "\r\n\\+CRSM: 144,0,\"([0-9AFaf]+)\"\r\n\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	/* cutting Cell ID number from the reply */
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

/*-------------------------------------------------------------------------*/

int at_change_pin(modem_t* modem, const char* old_pin, const char* new_pin)
{
	at_queue_t* at_q;
	at_query_t* q;
	char cmd[0x100];
	int res;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	snprintf(cmd, sizeof(cmd), "AT+CPWD=\"SC\",\"%s\",\"%s\"\r\n", old_pin, new_pin);

	q = at_query_create(cmd, "\r\nOK\r\n");
	at_query_exec(at_q->queue, q);

	res = at_query_is_error(q);

	at_query_free(q);

	return(res);
}

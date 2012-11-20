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

int at_raw_ok(modem_t* modem, const char* cmd)
{
	at_queue_t* at_q;
	at_query_t* q;
	int res = -1;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create(cmd, "\r\nOK\r\n");
	q->timeout = 10;
	at_query_exec(at_q->queue, q);

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
		nr = (nnr > MODEM_COPS_MODE_UNKNOWN && nnr < MODEM_COPS_MODE_COUNT)
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
	at_queue_t* at_q;
	at_query_t* q;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(res);

	q = at_query_create("AT+CGMR\r\n", "\r\n(.*)\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
	{
		re_strncpy(fw_info->firmware, sizeof(fw_info->firmware), q->result, q->pmatch + 1);

		/* don't known firmware creation date */
		fw_info->release = 0;

		res = fw_info;
	}

	at_query_free(q);

	return(res);
}

/*------------------------------------------------------------------------*/

char* at_get_operator_name(modem_t* modem, char* oper, size_t len)
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

/*------------------------------------------------------------------------*/

char* at_get_operator_number(modem_t* modem, char* oper_number, size_t len)
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

/*------------------------------------------------------------------------*/

int at_change_pin(modem_t* modem, const char* old_pin, const char* new_pin)
{
	char cmd[0x100];

	snprintf(cmd, sizeof(cmd), "AT+CPWD=\"SC\",\"%s\",\"%s\"\r\n", old_pin, new_pin);

	return(at_raw_ok(modem, cmd));
}

/*------------------------------------------------------------------------*/

int at_operator_select(modem_t* modem, int hni, modem_oper_act_t act)
{
	char cmd[0x100];

	if(hni)
		snprintf(cmd, sizeof(cmd), "AT+COPS=1,2,%d,%d\r\n", hni, act);
	else
	{
		strncpy(cmd, "AT+COPS=0\r\n", sizeof(cmd));
		cmd[sizeof(cmd) - 1] = 0;
	}

	return(at_raw_ok(modem, cmd));
}

/*------------------------------------------------------------------------*/

int at_set_wwan_profile(modem_t* modem, modem_data_profile_t* profile)
{
	char cmd[0x100];
	int res;

	if(profile->auth)
	{
		snprintf(cmd, sizeof(cmd), "AT$QCPDPP=3,%d,\"%s\",\"%s\"",
			profile->auth, profile->password, profile->username
		);
	}
	else
	{
		strncpy(cmd, "AT$QCPDPP=3,0", sizeof(cmd));
		cmd[sizeof(cmd) - 1] = 0;
	}

	/* setup authorization data */
	if((res = at_raw_ok(modem, cmd)))
		return(res);

	snprintf(cmd, sizeof(cmd), "AT+CGDCONT=3,\"IP\",\"%s\"", profile->apn);

	/* setup APN */
	return(res = at_raw_ok(modem, cmd));
}

/*------------------------------------------------------------------------*/

char* at_ussd_cmd(modem_t* modem, const char* query)
{
	at_queue_t* at_q = modem_proto_get(modem, MODEM_PROTO_AT);
	at_query_t *q;
	char s[0x100];
	char* res = NULL;

	if(!at_q)
		return(NULL);

	/* formating at command */
	snprintf(s, sizeof(s), "AT+CUSD=1,\"%s\",15\r\n", query);

	q = at_query_create(s, "\\+CUSD: ([0-9]{1}),\"(.+)\",15\r\n");
	q->timeout = 20;
	at_query_exec(at_q->queue, q);

	if(q->result)
	{
		printf("(II) USSD status = %d\n", re_atoi(q->result, q->pmatch + 1));
		res = re_strdup(q->result, q->pmatch + 2);
	}

	at_query_free(q);

	return(res);
}

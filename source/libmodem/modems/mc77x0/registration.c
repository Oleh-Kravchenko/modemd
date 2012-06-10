#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>

#include "modem/types.h"
#include "modem/modem_str.h"
#include "modem/modem_errno.h"

#include "at/at_queue.h"
#include "at/at_utils.h"
#include "at/at_common.h"

#include "modems/modem_conf.h"
#include "modems/mc77x0/registration.h"

#include "utils/str.h"
#include "utils/re.h"

/*------------------------------------------------------------------------*/

void modem_reset(modem_t* modem);

/*------------------------------------------------------------------------*/

#define __STR(x) #x

/*------------------------------------------------------------------------*/

enum registration_state_e
{
	RS_INIT = 0,
	RS_DISABLE_ECHO,
	RS_CMEE_NUMBER,
	RS_GET_FIRMWARE_VER,
	RS_GET_IMEI,
	RS_SET_CFUN,
	RS_READ_CONFIG,
	RS_SET_BAND,
	RS_SET_APN,
	RS_SET_AUTH,
	RS_CHECK_PIN,
	RS_SET_PIN,
	RS_SET_PUK,
	RS_GET_IMSI,
	RS_MCC_LOCK,
	RS_MNC_LOCK,
	RS_CCID_LOCK,
	RS_MSIN_LOCK,
	RS_OPERATOR_SELECT,
	RS_CHECK_REGISTRATION,
	RS_GET_SIGNAL_QUALITY,
	RS_GET_NETWORK_TYPE,
	RS_GET_OPERATOR_NAME,
	RS_RESET
};

static const char *RS_STR[] =
{
	__STR(RS_INIT),
	__STR(RS_DISABLE_ECHO),
	__STR(RS_CMEE_NUMBER),
	__STR(RS_GET_FIRMWARE_VER),
	__STR(RS_GET_IMEI),
	__STR(RS_SET_CFUN),
	__STR(RS_READ_CONFIG),
	__STR(RS_SET_BAND),
	__STR(RS_SET_APN),
	__STR(RS_SET_AUTH),
	__STR(RS_CHECK_PIN),
	__STR(RS_SET_PIN),
	__STR(RS_SET_PUK),
	__STR(RS_GET_IMSI),
	__STR(RS_MCC_LOCK),
	__STR(RS_MNC_LOCK),
	__STR(RS_CCID_LOCK),
	__STR(RS_MSIN_LOCK),
	__STR(RS_OPERATOR_SELECT),
	__STR(RS_CHECK_REGISTRATION),
	__STR(RS_GET_SIGNAL_QUALITY),
	__STR(RS_GET_NETWORK_TYPE),
	__STR(RS_GET_OPERATOR_NAME),
	__STR(RS_RESET)
};

/*------------------------------------------------------------------------*/

int at_get_signal_quality_mc7750(queue_t *queue, modem_signal_quality_t* sq)
{
	at_query_t *q;
	int res = -1, nrssi, nber;

	sq->dbm = 0;
	sq->level = 0;

	q = at_query_create("AT+CSQ\r\n", "([0-9]+), ?([0-9]+)\r\n\r\nOK\r\n");
	at_query_exec(queue, q);

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

modem_network_reg_t at_network_registration_mc7750(queue_t* queue)
{
	modem_network_reg_t nr = MODEM_NETWORK_REG_UNKNOWN;
	at_query_t *q;
	int nnr;

	q = at_query_create("AT+CEREG?\r\n", "\r\n\\+CEREG: [0-9],([0-9])\r\n\r\nOK\r\n");

	at_query_exec(queue, q);

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

void* mc77x0_thread_reg(modem_t *priv)
{
	enum registration_state_e state = RS_INIT;
	at_queue_t* at_q = priv->priv;
	int periodical_reset;
	int state_delay = 0;
	modem_conf_t conf;
	char s[0x100];

	int prev_last_error = 0;
	int cnt_last_error = 0;

	priv->reg.ready = 0;
	priv->reg.last_error = 258; /* we are busy now */
	priv->reg.state.reg = MODEM_NETWORK_REG_SEARCHING;

	while(!priv->reg.terminate)
	{
		/* delay for commands */
		if(state_delay)
		{
			/* periodical reset handling */
			if(state != RS_RESET && conf.periodical_reset)
			{
				if(periodical_reset == 0)
				{
					printf("Periodical reset modem..\n");

					periodical_reset = conf.periodical_reset * 3600;

					state = RS_RESET;
				}
				else
				{
					-- periodical_reset;

#ifdef _DEV_EDITION
					printf("Periodical reset modem: %d\n", periodical_reset);
#endif
				}
			}

			sleep(1);

#ifdef _DEV_EDITION
			printf("Delay: %d\n", state_delay);
#endif
			-- state_delay;

			continue;
		}
		else
		{
			if(at_q->last_error != -1 && at_q->last_error == prev_last_error)
			{
				++ cnt_last_error;

#ifdef _DEV_EDITION
				printf("cnt_last_error: %d\n", cnt_last_error);
#endif

				if(cnt_last_error > 10)
				{
					cnt_last_error = 0;
					prev_last_error = at_q->last_error;

					printf("Too many errors (%d), reseting modem..\n", prev_last_error);

					state = RS_RESET;
				}

				state_delay = 1;
			}
			else
			{
				cnt_last_error = 0;
				prev_last_error = at_q->last_error;
			}
		}

#ifdef _DEV_EDITION
		printf("State: %s\n", RS_STR[state]);
		printf("last_error: %d\n", at_q->last_error);
#endif

		if(state == RS_INIT)
		{
			printf("Started registration for modem on port %s\n", priv->port);

			state = RS_DISABLE_ECHO;
		}
		else if(state == RS_DISABLE_ECHO)
		{
			at_raw_ok(at_q->queue, "ATE0\r\n");

			state = RS_CMEE_NUMBER;
		}
		else if(state == RS_CMEE_NUMBER)
		{
			at_raw_ok(at_q->queue, "AT+CMEE=1\r\n");

			state = RS_GET_FIRMWARE_VER;
		}
		else if(state == RS_GET_FIRMWARE_VER)
		{
			at_get_fw_version(at_q->queue, &priv->reg.state.fw_info);

			state = RS_GET_IMEI;
		}
		else if(state == RS_GET_IMEI)
		{
			at_get_imei(at_q->queue, priv->reg.state.imei, sizeof(priv->reg.state.imei));

			state = RS_SET_CFUN;
		}
		else if(state == RS_SET_CFUN)
		{
			if(at_raw_ok(at_q->queue, "AT+CFUN=1\r\n"))
			{
				state_delay = 5;

				continue;
			}

			state = RS_READ_CONFIG;
		}
		else if(state == RS_READ_CONFIG)
		{
			/* waiting for config */
			if(modem_conf_read(priv->port, &conf))
			{
				state_delay = 1;

				continue;
			}

			periodical_reset = conf.periodical_reset * 3600;
				
			state = RS_SET_BAND;
		}
		else if(state == RS_SET_BAND)
		{
			/* band selection */
			snprintf(s, sizeof(s), "AT!BAND=%02X\r\n", conf.frequency_band);
			at_raw_ok(at_q->queue, s);

			state = RS_CHECK_PIN; //RS_SET_APN; //@Anatoly - do not set PDP profile on MC7750, during registration
		}
#if 0 //@Anatoly - do not set PDP profile on MC7750, during registration
		else if(state == RS_SET_APN)
		{
			/* apn setup */
			if(*conf.data.apn)
			{
				snprintf(s, sizeof(s), "AT+CGDCONT=3,\"IPV4V6\",\"%s\"\r\n", conf.data.apn);
				at_raw_ok(at_q->queue, s);
				
				state = RS_SET_AUTH;
			}
			else
				state = RS_CHECK_PIN;
		}
		else if(state == RS_SET_AUTH)
		{
			/* autorization data */
			if(conf.data.auth != PPP_NONE)
			{
				snprintf(s, sizeof(s), "AT$QCPDPP=3,%d,\"%s\",\"%s\"\r\n",
					conf.data.auth, conf.data.password, conf.data.username);

				at_raw_ok(at_q->queue, s);
			}
			else
			{
				at_raw_ok(at_q->queue, "AT$QCPDPP=3,0\r\n");
			}

			state = RS_CHECK_PIN;
		}
#endif
		else if(state == RS_CHECK_PIN)
		{
			switch(at_cpin_state(at_q->queue))
			{
				case MODEM_CPIN_STATE_PIN:
					state = RS_SET_PIN;
					break;

				case MODEM_CPIN_STATE_PUK:
					state = RS_SET_PUK;
					break;

				case MODEM_CPIN_STATE_READY:
					state = RS_GET_IMSI;
					break;

				default:
					if(at_q->last_error == __ME_SIM_BUSY)
						state_delay = 4;
					else if(at_q->last_error == __ME_SIM_WRONG)
                        return(NULL);
			}
		}
		else if(state == RS_SET_PIN)
		{
			if(*conf.pin)
			{
				at_cpin_pin(at_q->queue, conf.pin);

				state = RS_GET_IMSI;
			}
			else
			{
				priv->reg.last_error = __ME_SIM_PIN; /* SIM PIN required */

				return(NULL);
			}
		}
		else if(state == RS_SET_PUK)
		{
			if(*conf.pin && *conf.puk)
			{
				at_cpin_puk(at_q->queue, conf.puk, conf.pin);

				state = RS_GET_IMSI;
			}
			else
			{
				priv->reg.last_error = __ME_SIM_PUK; /* SIM PUK required */

				return(NULL);
			}
		}
		else if(state == RS_GET_IMSI)
		{
			if(!at_get_imsi(at_q->queue, priv->reg.state.imsi, sizeof(priv->reg.state.imsi)))
			{
				/* SIM busy? */
				state_delay = 5;

				continue;
			}

			/* mcc */
			strncpy(priv->reg.state.mcc, priv->reg.state.imsi, sizeof(priv->reg.state.mcc) - 1);
			priv->reg.state.mcc[sizeof(priv->reg.state.mcc) - 1] = 0;

			/* mnc */
			strncpy(priv->reg.state.mnc, priv->reg.state.imsi + strlen(priv->reg.state.mcc), sizeof(priv->reg.state.mnc) - 1);
			priv->reg.state.mnc[mnc_get_length(priv->reg.state.imsi)] = 0;

			state = RS_MCC_LOCK;
		}
		else if(state == RS_MCC_LOCK)
		{
			printf("MCC = [%s]\n", priv->reg.state.mcc);

			if(*conf.mcc_lock)
			{
				if(strcmp(priv->reg.state.mcc, conf.mcc_lock) != 0)
				{
					/* Network not allowed, emergency calls only  */
					priv->reg.last_error = __ME_MCC_LOCKED;

					/* set registration status as a denied */
					priv->reg.state.reg = MODEM_NETWORK_REG_DENIED;

					printf("(EE) MCC Lock error\n");

					return(NULL);
				}
			}

			state = RS_MNC_LOCK;
		}
		else if(state == RS_MNC_LOCK)
		{
			printf("MNC = [%s]\n", priv->reg.state.mnc);

			if(*conf.mnc_lock)
			{
				if(strcmp(priv->reg.state.mnc, conf.mnc_lock) != 0)
				{
					/* Network not allowed, emergency calls only  */
					priv->reg.last_error = __ME_MNC_LOCKED;

					/* set registration status as a denied */
					priv->reg.state.reg = MODEM_NETWORK_REG_DENIED;

					printf("(EE) MNC Lock error\n");

					return(NULL);
				}
			}

			state = RS_CCID_LOCK;
		}
		else if(state == RS_CCID_LOCK)
		{
			if(!at_get_ccid(at_q->queue, priv->reg.state.ccid, sizeof(priv->reg.state.ccid)))
			{
				state_delay = 5;

				continue;
			}

			printf("CCID = [%s]\n", priv->reg.state.ccid);

			if
			(
				*conf.ccid.low && *conf.ccid.high && (
					strcasecmp(priv->reg.state.ccid, conf.ccid.low) < 0 ||
					strcasecmp(conf.ccid.high, priv->reg.state.ccid) < 0
				)
			)
			{
				/* Network not allowed, emergency calls only  */
				priv->reg.last_error = __ME_CCID_LOCKED;

				/* set registration status as a denied */
				priv->reg.state.reg = MODEM_NETWORK_REG_DENIED;

				printf("(EE) CCID Lock error\n");

				return(NULL);
			}

			state = RS_MSIN_LOCK;
		}
		else if(state == RS_MSIN_LOCK)
		{
			strncpy(priv->reg.state.msin,
				priv->reg.state.imsi +
					strlen(priv->reg.state.mcc) +
					strlen(priv->reg.state.mnc),
				sizeof(priv->reg.state.msin));

			printf("MSIN = [%s]\n", priv->reg.state.msin);

			if
			(
				*conf.msin.low && *conf.msin.high && (
					strcasecmp(priv->reg.state.msin, conf.msin.low) < 0 ||
					strcasecmp(conf.msin.high, priv->reg.state.msin) < 0
				)
			)
			{
				/* Network not allowed, emergency calls only  */
				priv->reg.last_error = __ME_MSIN_LOCKED;

				/* set registration status as a denied */
				priv->reg.state.reg = MODEM_NETWORK_REG_DENIED;

				printf("(EE) MSIN Lock error\n");

				return(NULL);
			}

			state = RS_OPERATOR_SELECT;
		}
		else if(state == RS_OPERATOR_SELECT)
		{
			if(conf.operator_number)
			{
				snprintf(s, sizeof(s), "AT+COPS=1,2,%d,%d\r\n", conf.operator_number, conf.access_technology);
				at_raw_ok(at_q->queue, s);
			}
			else
				at_raw_ok(at_q->queue, "AT+COPS=0\r\n");

			state = RS_CHECK_REGISTRATION;
		}
		else if(state == RS_CHECK_REGISTRATION)
		{
			priv->reg.state.reg = at_network_registration_mc7750(at_q->queue);

//			printf("%s\n", str_network_registration(priv->reg.state.reg));

			/* if roaming disabled */
			if(MODEM_NETWORK_REG_ROAMING == priv->reg.state.reg && !conf.roaming)
				/* set registration status as a denied */
				priv->reg.state.reg = MODEM_NETWORK_REG_DENIED;

			switch(priv->reg.state.reg)
			{
				case MODEM_NETWORK_REG_HOME:
				case MODEM_NETWORK_REG_ROAMING:
					priv->reg.ready = 1;
					priv->reg.last_error = -1;
					state = RS_GET_SIGNAL_QUALITY;
					break;

				default:
					priv->reg.ready = 0;
					priv->reg.last_error = 0;

					state_delay = 5;
					break;
			}
		}
		else if(state == RS_GET_SIGNAL_QUALITY)
		{
			at_get_signal_quality_mc7750(at_q->queue, &priv->reg.state.sq);

			state = RS_GET_NETWORK_TYPE;
		}
		else if(state == RS_GET_NETWORK_TYPE)
		{
			at_get_network_type(at_q->queue, priv->reg.state.network_type, sizeof(priv->reg.state.network_type));

			state = RS_GET_OPERATOR_NAME;
		}
		else if(state == RS_GET_OPERATOR_NAME)
		{
			at_get_operator_name(at_q->queue, priv->reg.state.oper, sizeof(priv->reg.state.oper));

			state = RS_CHECK_REGISTRATION;

			state_delay = 10;
		}
		else if(state == RS_RESET)
		{
			modem_reset(priv);

			state = RS_INIT;
		}
	}

	return(NULL);
}

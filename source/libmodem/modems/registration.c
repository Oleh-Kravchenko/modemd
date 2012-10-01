#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <strings.h>

#include "modem/types.h"
#include "modem/modem_str.h"
#include "modem/modem_errno.h"

#include "utils/str.h"
#include "utils/re.h"

#include "proto.h"

#include "at/at_queue.h"
#include "at/at_utils.h"
#include "at/at_common.h"

#include "modems/modem_conf.h"
#include "modems/registration.h"

#include "modems/mc77x0/at_func.h"

/*------------------------------------------------------------------------*/

void modem_reset(modem_t* modem);

/*------------------------------------------------------------------------*/

enum registration_state_e
{
	RS_INIT = 0,
	RS_DISABLE_ECHO,
	RS_CMEE_NUMBER,
	RS_GET_FIRMWARE_VER,
	RS_GET_IMEI,
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
	RS_GET_OPERATOR_NUMBER,
	RS_GET_OPERATOR_NAME,
	RS_RESET
};

static const char *RS_STR[] =
{
#define __STR(x) #x
	__STR(RS_INIT),
	__STR(RS_DISABLE_ECHO),
	__STR(RS_CMEE_NUMBER),
	__STR(RS_GET_FIRMWARE_VER),
	__STR(RS_GET_IMEI),
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
	__STR(RS_GET_OPERATOR_NUMBER),
	__STR(RS_GET_OPERATOR_NAME),
	__STR(RS_RESET)
#undef __STR
};

/*------------------------------------------------------------------------*/

void* mc77x0_thread_reg(modem_t *priv)
{
	enum registration_state_e state = RS_INIT;
	at_queue_t* at_q = modem_proto_get(priv, MODEM_PROTO_AT);
	const modem_info_device_t* mdd = priv->mdd;
	int periodical_reset;
	int state_delay = 0;
	modem_conf_t conf;

	int last_error;
	int prev_last_error = 0;
	int cnt_last_error = 0;

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

		last_error = modem_queues_last_error(priv, MODEM_PROTO_AT);

		/* I/O modem error handling */
		if(last_error == __ME_WRITE_FAILED || last_error == __ME_READ_FAILED)
		{
			++ cnt_last_error;

#if _DEV_EDITION
			printf("cnt_last_error: %d\n", cnt_last_error);
#endif

			if(cnt_last_error > 3)
			{
				cnt_last_error = 0;
				prev_last_error = at_q->last_error;

				printf("(EE) Too many errors (%d), reseting modem..\n", prev_last_error);

				state = RS_RESET;
			}

			state_delay = 1;
		}
		else
		{
			cnt_last_error = 0;
			prev_last_error = at_q->last_error;
		}

#ifdef _DEV_EDITION
		printf("State: %s\n", RS_STR[state]);
		printf("last_error: %d\n", at_q->last_error);
#endif

		if(state == RS_INIT)
		{
			printf("Started registration for modem on port %s\n", priv->port);

			/* initialize data */
			priv->reg.last_error = __ME_REG_IN_PROGRESS;
			priv->reg.state.reg = MODEM_NETWORK_REG_SEARCHING;

			state = RS_DISABLE_ECHO;
		}
		else if(state == RS_DISABLE_ECHO)
		{
			at_raw_ok(priv, "ATE0\r\n");

			state = RS_CMEE_NUMBER;
		}
		else if(state == RS_CMEE_NUMBER)
		{
			at_raw_ok(priv, "AT+CMEE=1\r\n");

			state = RS_GET_FIRMWARE_VER;
		}
		else if(state == RS_GET_FIRMWARE_VER)
		{
			mdd->functions.get_fw_version(priv, &priv->reg.state.fw_info);

			state = RS_GET_IMEI;
		}
		else if(state == RS_GET_IMEI)
		{
			mdd->functions.get_imei(priv, priv->reg.state.imei, sizeof(priv->reg.state.imei));

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
			/* current modem band */
			int band = mdd->functions.get_freq_band(priv);
			
			printf("(II) Modem Band is %x\n", band);

			if(conf.frequency_band != band)
			{
				/* band selection */
				if(mdd->functions.set_freq_band(priv, conf.frequency_band))
				{
					priv->reg.last_error = at_q->last_error;

					printf("(EE) Band selection error: %d\n", at_q->last_error);

					return(NULL);
				}

				/* delay for band setup */
				state_delay = 5;

				/* reset is required */
				state = RS_RESET;

				continue;
			}

			state = RS_CHECK_PIN;
		}
		else if(state == RS_CHECK_PIN)
		{
			switch(mdd->functions.cpin_state(priv))
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
					/* registration error? */;
					priv->reg.last_error = at_q->last_error;

					if(at_q->last_error == __ME_SIM_BUSY)
						state_delay = 5;
					else if(at_q->last_error == __ME_SIM_WRONG)
						/* buggy MC7750 ... */
						state_delay = 10;
					else if(at_q->last_error == __ME_NO_SIM)
					{
						/* set registration status as a denied */
						priv->reg.state.reg = MODEM_NETWORK_REG_DENIED;

						return(NULL);
					}
			}
		}
		else if(state == RS_SET_PIN)
		{
			if(*conf.pin && !mdd->functions.cpin_pin(priv, conf.pin))
			{
				state = RS_GET_IMSI;
			}
			else
			{
				printf("(EE) PIN Error\n");

				if(*conf.pin)
					/* SIM PIN invalid */
					priv->reg.last_error = at_q->last_error;
				else
					priv->reg.last_error = __ME_SIM_PIN;

				/* set registration status as a denied */
				priv->reg.state.reg = MODEM_NETWORK_REG_DENIED;

				return(NULL);
			}
		}
		else if(state == RS_SET_PUK)
		{
			if(*conf.pin && *conf.puk && !mdd->functions.cpin_puk(priv, conf.puk, conf.pin))
			{
				state = RS_GET_IMSI;
			}
			else
			{
				if(*conf.pin && *conf.puk)
					/* SIM PUK invalid */
					priv->reg.last_error = at_q->last_error;
				else
					priv->reg.last_error = __ME_SIM_PUK;

				/* set registration status as a denied */
				priv->reg.state.reg = MODEM_NETWORK_REG_DENIED;

				return(NULL);
			}
		}
		else if(state == RS_GET_IMSI)
		{
			if(conf.frequency_band == 10)
			{
				state = RS_OPERATOR_SELECT;

				printf("(WW) Band is CDMA skip SIM Check's\n");

				continue;
			}

			if(!mdd->functions.get_imsi(priv, priv->reg.state.imsi, sizeof(priv->reg.state.imsi)))
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
			if(!mc77x0_at_get_ccid(priv, priv->reg.state.ccid, sizeof(priv->reg.state.ccid)))
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
			if(mdd->functions.operator_select(priv, conf.operator_number, conf.access_technology))
			{
				// operator selection is failed, wait 5 seconds and try again
				state_delay = 5;

				continue;
			}

			state = RS_CHECK_REGISTRATION;
		}
		else if(state == RS_CHECK_REGISTRATION)
		{
			priv->reg.state.reg = mdd->functions.network_registration(priv);

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
					priv->reg.last_error = __ME_REG_IN_PROGRESS;

					state_delay = 5;
					break;
			}
		}
		else if(state == RS_GET_SIGNAL_QUALITY)
		{
			mdd->functions.get_signal_quality(priv, &priv->reg.state.sq);

			state = RS_GET_NETWORK_TYPE;
		}
		else if(state == RS_GET_NETWORK_TYPE)
		{
			mdd->functions.get_network_type(priv, priv->reg.state.network_type, sizeof(priv->reg.state.network_type));

			state = RS_GET_OPERATOR_NUMBER;
		}
		else if(state == RS_GET_OPERATOR_NUMBER)
		{
			if(!mdd->functions.get_operator_number(priv, priv->reg.state.oper_number, sizeof(priv->reg.state.oper_number)))
				*priv->reg.state.oper_number = 0;

			state = RS_GET_OPERATOR_NAME;
		}
		else if(state == RS_GET_OPERATOR_NAME)
		{
			if(!mdd->functions.get_operator_name(priv, priv->reg.state.oper, sizeof(priv->reg.state.oper)))
				*priv->reg.state.oper = 0;

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

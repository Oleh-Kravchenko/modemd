#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "modem_conf.h"

#include "utils/str.h"
#include "utils/re.h"

/*------------------------------------------------------------------------*/

int modem_conf_read(const char* port, modem_conf_t* conf)
{
	char s[0x100], w[0x100];
	regmatch_t* pmatch;
	size_t nmatch;
	FILE *f;

	/* default values */
	*conf->pin = 0;
	*conf->puk = 0;
	*conf->data.apn = 0;
	*conf->data.apn = 0;
	conf->data.auth = PPP_NONE;
	*conf->data.username = 0;
	*conf->data.password = 0;
	conf->roaming = 0;
	conf->operator_number = 0;
	conf->access_technology = 0;
	conf->frequency_band = 0;
	conf->periodical_reset = 0;
	*conf->mcc_lock = 0;
	*conf->mnc_lock = 0;
	*conf->ccid.low = 0;
	*conf->ccid.high = 0;

	/* path to config */
	snprintf(s, sizeof(s), "/etc/modemd/%s/conf", port);

	if(!(f = fopen(s, "r")))
	{
		printf("(DD) Waiting configuration for %s port. ", port);
		perror(s);
		return(-1);
	}

	while(!feof(f))
	{
		if(!fgets(s, sizeof(s), f))
			continue;

#define CONF_PIN			"pin="
#define CONF_PUK			"puk="
#define CONF_APN			"apn="
#define CONF_AUTH			"auth="
#define CONF_USER			"username="
#define CONF_PASS			"password="
#define CONF_ROAMING		"roaming_enable=yes"
#define CONF_OPER			"operator_number="
#define CONF_ACT			"access_technology="
#define CONF_BAND			"frequency_band="
#define CONF_PERIODICAL_RST	"periodical_reset="
#define CONF_MCC			"mcc="
#define CONF_MNC			"mnc="
#define CONF_CCID			"ccid="
#define CONF_MSIN			"msin="

		if(strstr(s, CONF_PIN) == s)
		{
			strncpy(conf->pin, s + strlen(CONF_PIN), sizeof(conf->pin) - 1);
			conf->pin[sizeof(conf->pin) - 1] = 0;
			trim_r(conf->pin);
		}
		else if(strstr(s, CONF_PUK) == s)
		{
			strncpy(conf->puk, s + strlen(CONF_PUK), sizeof(conf->puk) - 1);
			conf->puk[sizeof(conf->puk) - 1] = 0;
			trim_r(conf->puk);
		}
		else if(strstr(s, CONF_APN) == s)
		{
			strncpy(conf->data.apn, s + strlen(CONF_APN), sizeof(conf->data.apn) - 1);
			conf->data.apn[sizeof(conf->data.apn) - 1] = 0;
			trim_r(conf->data.apn);
		}
		else if(strstr(s, CONF_AUTH) == s)
		{
			conf->data.auth = atoi(s + strlen(CONF_AUTH));
		}
		else if(strstr(s, CONF_USER) == s)
		{
			strncpy(conf->data.username, s + strlen(CONF_USER), sizeof(conf->data.username) - 1);
			conf->data.username[sizeof(conf->data.username) - 1] = 0;
			trim_r(conf->data.username);
		}
		else if(strstr(s, CONF_PASS) == s)
		{
			strncpy(conf->data.password, s + strlen(CONF_PASS), sizeof(conf->data.password) - 1);
			conf->data.password[sizeof(conf->data.password) - 1] = 0;
			trim_r(conf->data.password);
		}
		else if(strstr(s, CONF_ROAMING))
		{
			conf->roaming = 1;
		}
		else if(strstr(s, CONF_OPER) == s)
		{
			conf->operator_number = atoi(s + strlen(CONF_OPER));
		}
		else if(strstr(s, CONF_ACT) == s)
		{
			conf->access_technology = atoi(s + strlen(CONF_ACT));
		}
		else if(strstr(s, CONF_BAND) == s)
		{
			sscanf(s + strlen(CONF_BAND), "%x", (unsigned int*)&conf->frequency_band);
		}
		else if(strstr(s, CONF_PERIODICAL_RST) == s)
		{
			conf->periodical_reset = atoi(s + strlen(CONF_PERIODICAL_RST));
		}
		else if(strstr(s, CONF_MCC) == s)
		{
			strncpy(conf->mcc_lock, s + strlen(CONF_MCC), sizeof(conf->mcc_lock) - 1);
			conf->mcc_lock[sizeof(conf->mcc_lock) - 1] = 0;
			trim_r(conf->mcc_lock);
		}
		else if(strstr(s, CONF_MNC) == s)
		{
			strncpy(conf->mnc_lock, s + strlen(CONF_MCC), sizeof(conf->mnc_lock) - 1);
			conf->mnc_lock[sizeof(conf->mnc_lock) - 1] = 0;
			trim_r(conf->mnc_lock);
		}
		else if(strstr(s, CONF_CCID) == s)
		{
			strncpy(w, s + strlen(CONF_CCID), sizeof(w) - 1);
			w[sizeof(w) - 1] = 0;
			trim_r(w);

			if(!re_parse(w, "(.+),(.+)", &nmatch, &pmatch))
			{
				re_strncpy(conf->ccid.low, sizeof(conf->ccid.low), w, pmatch + 1);
				re_strncpy(conf->ccid.high, sizeof(conf->ccid.low), w, pmatch + 2);

				free(pmatch);
			}
		}
		else if(strstr(s, CONF_MSIN) == s)
		{
			strncpy(w, s + strlen(CONF_MSIN), sizeof(w) - 1);
			w[sizeof(w) - 1] = 0;
			trim_r(w);

			if(!re_parse(w, "(.+),(.+)", &nmatch, &pmatch))
			{
				re_strncpy(conf->msin.low, sizeof(conf->msin.low), w, pmatch + 1);
				re_strncpy(conf->msin.high, sizeof(conf->msin.low), w, pmatch + 2);

				free(pmatch);
			}
		}
	}

	fclose(f);

	printf(
		"              PIN: [%s]\n"
		"              PUK: [%s]\n"
		"              APN: [%s]\n"
		"             Auth: %d\n"
		"         Username: [%s]\n"
		"         Password: [%s]\n"
		"          roaming: %d\n"
		"  operator_number: %d\n"
		"access_technology: %d\n"
		" periodical_reset: %d\n"
		"   frequency_band: %d\n"
		"         mcc lock: [%s]\n"
		"         mnc lock: [%s]\n"
		"        ccid lock: [%s] - [%s]\n"
		"        msin lock: [%s] - [%s]\n",

		conf->pin,
		conf->puk,
		conf->data.apn,
		conf->data.auth,
		conf->data.username,
		conf->data.password,
		conf->roaming,
		conf->operator_number,
		conf->access_technology,
		conf->periodical_reset,
		conf->frequency_band,
		conf->mcc_lock,
		conf->mnc_lock,
		conf->ccid.low, conf->ccid.high,
		conf->msin.low, conf->msin.high
	);

	return(0);
}

#include <string.h>
#include <stdlib.h>

#include <modem/types.h>

#include "utils/file.h"
#include "utils/re.h"

/*------------------------------------------------------------------------*/

int at_parse_cops_list(const char* s, modem_oper_t** opers)
{
    regmatch_t *pmatch;
    modem_oper_t* oper;
	size_t nmatch;
    int nopers;

    nopers = 0;
    *opers = NULL;

    while(*s)
    {
        if(regmatch_parse(s, "\\(([0123]),\"(.{0,16})\",\"(.{0,16})\",\"([0-9]{5,16})\",([012])),?", &nmatch, &pmatch))
			break;

        /* increase memory for result */
        *opers = realloc(*opers, sizeof(modem_oper_t) * (nopers + 1));

        oper = (*opers + nopers);

        /* add new info about operator */
        oper->stat = regmatch_atoi(s, pmatch + 1);
        regmatch_ncpy(oper->longname, sizeof(oper->longname), s, pmatch + 2);
        regmatch_ncpy(oper->shortname, sizeof(oper->shortname), s, pmatch + 3);
        regmatch_ncpy(oper->numeric, sizeof(oper->numeric), s, pmatch + 4);
        oper->act = regmatch_atoi(s, pmatch + 5);

        ++ nopers;

        /* next operator to parse */
        s += pmatch->rm_eo - pmatch->rm_so;

		free(pmatch);
    }

    return(nopers);
}

/*------------------------------------------------------------------------*/

int at_parse_error(const char* s)
{
	regmatch_t* pmatch;
	size_t nmatch;
    int cme_error;

    if(strstr(s, "\r\nERROR\r\n"))
        /* modem failure (general error) or reporting is AT+CMEE=0 */
        return(0);

	if(regmatch_parse(s, "\\+CME ERROR: ([0-9]{1,5})\r\n", &nmatch, &pmatch))
        return(0);

    /* getting parsed integer value of CME error */
    cme_error = regmatch_atoi(s, pmatch + 1);

    free(pmatch);

    return(cme_error);
}

/*------------------------------------------------------------------------*/

int mnc_get_length(const char *imsi)
{
#define MCC_LEN 3
#define MAX_MNC_LEN 3

	int nspec_mcc[] = {302, 310, 311, 316, 334, 338, 342, 344, 346, 348, 365, 374, 376, 708, 722, 732, 0};
	char mcc[MCC_LEN + 1];
	int i = 0, found = 0, ret;
	int nmcc;
	// If MCC belongs to special MCC group, check if it's a "super special MCC" :-)
	char mnc[MAX_MNC_LEN + 1];

	//! IMSI value (15 digits), starting with MCC (3 digits) / MNC (2 digits, 3 for PCS 1900)
	//! Mobile operators with MCC listed in 'iSpecialMCC' has 3 digits in MNC
	//! Mobile operators with MCC 302 or 374 may have MNC with 2 or 3 digits
	//! For existing MCCs and MNCs see the "MOBILE NETWORK CODES (MNC) FOR THE INTERNATIONAL
	//! IDENTIFICATION PLAN FOR PUBLIC NETWORKS AND SUBSCRIPTIONS" document
	//! of TELECOMMUNICATION STANDARDIZATION BUREAU OF International Telecommunication Union


	//! Extract MCC
	strncpy( mcc, imsi, MCC_LEN );
	mcc[MCC_LEN] = 0;

	nmcc = atoi(mcc);

	//! Check if MCC belogs to special MCC group (3 digits MNC)
	while(nspec_mcc[i] && !found)
	{
		if(nmcc == nspec_mcc[i])
		{
			found = 1;
			break;
		}

		++ i;
	}

	if(!found)
	{
		ret = 2;
		goto exit;
	}

	switch(nmcc)
	{
		case 302:
			strncpy(mnc, imsi + MCC_LEN, 3 );
			mnc[MCC_LEN] = 0;

			ret = (atoi(mnc) == 656 ? 2 : 3);
			break;

		case 374:
			strncpy( mnc, imsi + MCC_LEN, 3 );
			mnc[MCC_LEN] = 0;

			ret = (atoi(mnc) == 12 ? 2 : 3);
			break;

		default:
			ret = 3;
	}

exit:
	return ret;
}

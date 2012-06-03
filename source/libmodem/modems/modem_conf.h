#ifndef __MODEM_CONF_H
#define __MODEM_CONF_H

#include "modem/types.h"

typedef struct
{
	char pin[20];

	char puk[20];

	modem_data_profile_t data;

	int roaming;

	int operator_number;

	int access_technology;

	int frequency_band;
	
	/* time in hours */
	int periodical_reset;
} modem_conf_t;

/*------------------------------------------------------------------------*/

int modem_conf_read(const char* port, modem_conf_t* conf);

#endif /* __MODEM_CONF_H */

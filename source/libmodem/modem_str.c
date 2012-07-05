#include "modem/modem_str.h"

/*------------------------------------------------------------------------*/

const char* str_network_registration(modem_network_reg_t nr)
{
	static const char* network_registration[] = {
		"Not registered, modem is not currently searching a new operator to register to",
		"Registered, home network",
		"Not registered, but modem is currently searching a new operator to register to",
		"Registration denied",
		"Unknown",
		"Registered, roaming",
	};

	int i = nr;

	/* check and return status of registration */
	return(network_registration[(i >= 0 && i <= 5) ? nr : MODEM_NETWORK_REG_UNKNOWN]);
}

/*------------------------------------------------------------------------*/

const char* str_proto(modem_proto_t proto)
{
	static const char* protocol_name[] = {
		"None",
		"AT",
		"QCQMI",
	};

	/* check and return name of protocol */
	return(protocol_name[(proto >= 0 && proto < 3) ? proto : 0]);
}

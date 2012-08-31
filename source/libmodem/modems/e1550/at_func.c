#include <stdio.h>
#include <unistd.h>

#include <modem/types.h>

#include "proto.h"

#include "at/at_queue.h"
#include "at/at_common.h"

#include "utils/re.h"
#include "utils/str.h"

/*------------------------------------------------------------------------*/

char* e1550_at_get_network_type(modem_t* modem, char* network, size_t len)
{
	static const char* sys_submode_str[] = {
		"No service",
		"GSM",
		"GPRS",
		"EDGE",
		"WCDMA",
		"HSDPA",
		"HSUPA",
		"HSDPA+HSUPA",
		"TD_SCDMA",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+",
		"HSPA+ (64QAM)",
		"HSPA+ (MIMO)",
	};

	int sys_submode = 0;
	at_queue_t* at_q;
	at_query_t* q;

	if(!(at_q = modem_proto_get(modem, MODEM_PROTO_AT)))
		return(NULL);

	q = at_query_create("AT^SYSINFO\r\n", "\r\n\\^SYSINFO:[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]*,([0-9]+)\r\n\r\nOK\r\n");

	at_query_exec(at_q->queue, q);

	if(!at_query_is_error(q))
		sys_submode = re_atoi(q->result, q->pmatch + 1);

	at_query_free(q);

	if(!sys_submode)
		return(NULL);

	/* when the returned value of <sys_submode> is out of (0~18),
		you should treat it as 4 (WCDMA) by default */
	if(sys_submode < 0 || sys_submode > 18)
		sys_submode = 4;

	strncpy(network, sys_submode_str[sys_submode], len - 1);
	network[len - 1] = 0;

	return(network);
}

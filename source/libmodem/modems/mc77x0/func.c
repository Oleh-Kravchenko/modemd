#include <modem/types.h>

#include "proto/at/at_queue.h"
#include "proto/at/at_common.h"

/*------------------------------------------------------------------------*/

int mc77x0_get_cell_id(modem_t* modem)
{
	at_queue_t* q = modem->priv;

	return(at_get_cell_id(q->queue));
}

/*------------------------------------------------------------------------*/

time_t mc77x0_get_network_time(modem_t* modem)
{
	at_queue_t* q = modem->priv;

	return(at_get_network_time(q->queue));
}

/*------------------------------------------------------------------------*/

int mc77x0_operator_scan(modem_t* modem, modem_oper_t** opers)
{
	at_queue_t* q = modem->priv;

	return(at_operator_scan(q->queue, opers));
}

/*------------------------------------------------------------------------*/

int mc77x0_change_pin(modem_t* modem, const char* old_pin, const char* new_pin)
{
	at_queue_t* at_q = modem->priv;

	return(at_change_pin(at_q->queue, old_pin, new_pin));
}

#ifndef __MODEM_PROTO_H
#define __MODEM_PROTO_H

#include <modem/types.h>

#include "modem_info.h"

/*------------------------------------------------------------------------*/

int modem_queues_init(modem_t* modem);

void modem_queues_destroy(modem_t* modem);

void modem_queues_suspend(modem_t* modem);

void modem_queues_resume(modem_t* modem);

void* modem_proto_get(modem_t* modem, modem_proto_t proto);

int modem_queues_add(modem_t* modem, modem_proto_t proto, void* queue);

int modem_queues_last_error(modem_t* modem, modem_proto_t proto);

#endif /* __MODEM_PROTO_H */

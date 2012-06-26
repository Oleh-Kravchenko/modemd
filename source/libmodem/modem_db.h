#ifndef __MODEM_DB_H
#define __MODEM_DB_H

#include <stdint.h>

#include "modem/types.h"

/*------------------------------------------------------------------------*/

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

/*------------------------------------------------------------------------*/

typedef void *(*registration_func_t)(modem_t*);

typedef void *(*pthread_func_t)(void*);

/*------------------------------------------------------------------------*/

typedef struct
{
	int num;

	modem_proto_t type;
} modem_iface_t;

/*------------------------------------------------------------------------*/

typedef struct
{
	char product[0x100];

	uint16_t vendor_id;

	uint16_t product_id;

	registration_func_t thread_reg;

	modem_iface_t iface[__MODEM_IFACE_MAX];
} modem_db_device_t;
 
/*------------------------------------------------------------------------*/

extern modem_db_device_t modem_db_devices[];

/*------------------------------------------------------------------------*/

extern const int modem_db_devices_cnt;

#endif /* __MODEM_DB_H */

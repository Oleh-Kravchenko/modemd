#ifndef __MODEM_DB_H
#define __MODEM_DB_H

#include <stdint.h>

#include "modem/types.h"

/*------------------------------------------------------------------------*/

#define __MODEM_IFACE_MAX 4

/*------------------------------------------------------------------------*/

typedef void *(*registration_func_t)(modem_t*);

typedef void *(*pthread_func_t)(void*);

/*------------------------------------------------------------------------*/

typedef enum
{
    MODEM_PROTO_NONE	= 0,
    MODEM_PROTO_AT,
    MODEM_PROTO_CNS,
    MODEM_PROTO_WAN
} modem_proto_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    int num;

    modem_proto_t type;
} modem_iface_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    uint16_t vendor;

    uint16_t product;

    registration_func_t thread_reg;

    modem_iface_t iface[__MODEM_IFACE_MAX];
} modem_db_device_t;
 
/*------------------------------------------------------------------------*/

extern modem_db_device_t modem_db_devices[];

/*------------------------------------------------------------------------*/

extern const int modem_db_devices_cnt;

#endif /* __MODEM_DB_H */

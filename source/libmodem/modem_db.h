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

typedef char* (*modem_get_imei_func_t)(modem_t* modem, char* imei, int len);

typedef int (*modem_get_signal_quality_func_t)(modem_t* modem, modem_signal_quality_t* sq);

typedef time_t (*modem_get_network_time_func_t)(modem_t* modem);

typedef char* (*modem_get_imsi_func_t)(modem_t* modem, char* imsi, int len);

typedef char* (*modem_get_operator_name_func_t)(modem_t* modem, char *oper, int len);

typedef modem_network_reg_t (*modem_network_registration_func_t)(modem_t* modem);

typedef char* (*modem_get_network_type_func_t)(modem_t* modem, char *network, int len);

typedef int (*modem_change_pin_func_t)(modem_t* modem, const char* old_pin, const char* new_pin);

typedef modem_fw_ver_t* (*modem_get_fw_version_func_t)(modem_t* modem, modem_fw_ver_t* fw_info);

typedef int (*modem_operator_scan_func_t)(modem_t* modem, modem_oper_t** opers);

typedef int (*modem_get_cell_id_func_t)(modem_t* modem);

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

	struct
	{
#define __MODEM_DB_FUNC(s) s##_func_t s

		__MODEM_DB_FUNC(modem_get_imei);
		__MODEM_DB_FUNC(modem_get_signal_quality);
		__MODEM_DB_FUNC(modem_get_network_time);
		__MODEM_DB_FUNC(modem_get_imsi);
		__MODEM_DB_FUNC(modem_get_operator_name);
		__MODEM_DB_FUNC(modem_network_registration);
		__MODEM_DB_FUNC(modem_get_network_type);
		__MODEM_DB_FUNC(modem_change_pin);
		__MODEM_DB_FUNC(modem_get_fw_version);
		__MODEM_DB_FUNC(modem_operator_scan);
		__MODEM_DB_FUNC(modem_get_cell_id);
	} func;

	registration_func_t thread_reg;

	modem_iface_t iface[__MODEM_IFACE_MAX];
} modem_db_device_t;
 
/*------------------------------------------------------------------------*/

extern modem_db_device_t modem_db_devices[];

/*------------------------------------------------------------------------*/

extern const int modem_db_devices_cnt;

#endif /* __MODEM_DB_H */

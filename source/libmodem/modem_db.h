#ifndef __MODEM_DB_H
#define __MODEM_DB_H

#include <stdint.h>

#include "modem/types.h"

/*------------------------------------------------------------------------*/

typedef void *(*registration_func_t)(modem_t*);

typedef void *(*pthread_func_t)(void*);

/*------------------------------------------------------------------------*/

typedef char* (*get_imei_func_t)(modem_t* modem, char* imei, size_t len);

typedef int (*get_signal_quality_func_t)(modem_t* modem, modem_signal_quality_t* sq);

typedef time_t (*get_network_time_func_t)(modem_t* modem);

typedef char* (*get_imsi_func_t)(modem_t* modem, char* imsi, size_t len);

typedef char* (*get_operator_name_func_t)(modem_t* modem, char* oper, size_t len);

typedef char* (*get_operator_number_func_t)(modem_t* modem, char* oper_number, size_t len);

typedef modem_network_reg_t (*network_registration_func_t)(modem_t* modem);

typedef char* (*get_network_type_func_t)(modem_t* modem, char* network, size_t len);

typedef int (*change_pin_func_t)(modem_t* modem, const char* old_pin, const char* new_pin);

typedef modem_fw_ver_t* (*get_fw_version_func_t)(modem_t* modem, modem_fw_ver_t* fw_info);

typedef int (*operator_scan_func_t)(modem_t* modem, modem_oper_t** opers);

typedef int (*get_cell_id_func_t)(modem_t* modem);

typedef int (*operator_select_func_t)(modem_t* modem, int hni, modem_oper_act_t act);

/*------------------------------------------------------------------------*/

typedef struct
{
	char vendor[0x100];

	char product[0x100];

	uint16_t vendor_id;

	uint16_t product_id;

	struct
	{
		#define __MODEM_DB_FUNC(s) s##_func_t s
			__MODEM_DB_FUNC(get_imei);
			__MODEM_DB_FUNC(get_signal_quality);
			__MODEM_DB_FUNC(get_network_time);
			__MODEM_DB_FUNC(get_imsi);
			__MODEM_DB_FUNC(get_operator_name);
			__MODEM_DB_FUNC(network_registration);
			__MODEM_DB_FUNC(get_network_type);
			__MODEM_DB_FUNC(change_pin);
			__MODEM_DB_FUNC(get_fw_version);
			__MODEM_DB_FUNC(operator_scan);
			__MODEM_DB_FUNC(get_cell_id);
			__MODEM_DB_FUNC(operator_select);
			__MODEM_DB_FUNC(get_operator_number);
		#undef __MODEM_DB_FUNC
	} functions;

	registration_func_t thread_reg;

	struct
	{
		int num;

		modem_proto_t type;
	} iface[__MODEM_IFACE_MAX];
} modem_db_device_t;
 
/*------------------------------------------------------------------------*/

extern modem_db_device_t modem_db_devices[];

/*------------------------------------------------------------------------*/

extern const int modem_db_devices_cnt;

#endif /* __MODEM_DB_H */

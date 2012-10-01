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

typedef int (*set_wwan_profile_func_t)(modem_t* modem, modem_data_profile_t* profile);

typedef int (*start_wwan_func_t)(modem_t* modem);

typedef int (*stop_wwan_func_t)(modem_t* modem);

typedef modem_state_wwan_t (*state_wwan_func_t)(modem_t* modem);

typedef modem_cpin_state_t (*cpin_state_func_t)(modem_t* modem);

typedef int (*cpin_pin_func_t)(modem_t* modem, const char* pin);

typedef int (*cpin_puk_func_t)(modem_t* modem, const char* puk, const char* pin);

typedef int (*get_freq_bands_func_t)(modem_t* modem, freq_band_t** band_list);

typedef int (*get_freq_band_func_t)(modem_t* modem);

typedef int (*set_freq_band_func_t)(modem_t* modem, int band_index);

typedef char* (*ussd_cmd_func_t)(modem_t* modem, const char* query);

/*------------------------------------------------------------------------*/

typedef struct
{
	char vendor[0x100];

	char product[0x100];

	uint16_t vendor_id;

	uint16_t product_id;

	struct
	{
		#define __MODEM_INFO_FUNC(s) s##_func_t s
			__MODEM_INFO_FUNC(get_imei);
			__MODEM_INFO_FUNC(get_signal_quality);
			__MODEM_INFO_FUNC(get_network_time);
			__MODEM_INFO_FUNC(get_imsi);
			__MODEM_INFO_FUNC(get_operator_name);
			__MODEM_INFO_FUNC(network_registration);
			__MODEM_INFO_FUNC(get_network_type);
			__MODEM_INFO_FUNC(change_pin);
			__MODEM_INFO_FUNC(get_fw_version);
			__MODEM_INFO_FUNC(operator_scan);
			__MODEM_INFO_FUNC(get_cell_id);
			__MODEM_INFO_FUNC(operator_select);
			__MODEM_INFO_FUNC(get_operator_number);
			__MODEM_INFO_FUNC(set_wwan_profile);
			__MODEM_INFO_FUNC(start_wwan);
			__MODEM_INFO_FUNC(stop_wwan);
			__MODEM_INFO_FUNC(state_wwan);
			__MODEM_INFO_FUNC(cpin_state);
			__MODEM_INFO_FUNC(cpin_pin);
			__MODEM_INFO_FUNC(cpin_puk);
			__MODEM_INFO_FUNC(get_freq_bands);
			__MODEM_INFO_FUNC(get_freq_band);
			__MODEM_INFO_FUNC(set_freq_band);
			__MODEM_INFO_FUNC(ussd_cmd);
		#undef __MODEM_INFO_FUNC
	} functions;

	registration_func_t thread_reg;

	struct
	{
		int num;

		modem_proto_t type;
	} iface[__MODEM_IFACE_MAX];
} modem_info_device_t;
 
/*------------------------------------------------------------------------*/

extern modem_info_device_t modem_info_devices[];

/*------------------------------------------------------------------------*/

extern const int modem_info_devices_cnt;

#endif /* __MODEM_DB_H */

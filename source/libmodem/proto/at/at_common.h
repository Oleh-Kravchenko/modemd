#ifndef __AT_COMMON_H

#include <modem/types.h>

#include "queue.h"

/*------------------------------------------------------------------------*/

modem_cpin_state_t at_cpin_state(modem_t* modem);

/*------------------------------------------------------------------------*/

int at_cpin_pin(modem_t* modem, const char* pin);

/*------------------------------------------------------------------------*/

int at_cpin_puk(modem_t* modem, const char* puk, const char* pin);

/*------------------------------------------------------------------------*/

/**
 * @brief execute AT command on modem
 * @param modem modem that support AT queue
 * @param cmd AT command
 * @return zero if no errors
 */
int at_raw_ok(modem_t* modem, const char* cmd);

/*------------------------------------------------------------------------*/

char* at_get_imsi(modem_t* modem, char* imsi, size_t len);

/*------------------------------------------------------------------------*/

char* at_get_imei(modem_t* modem, char* imsi, size_t len);

/*------------------------------------------------------------------------*/

int at_operator_scan(modem_t* modem, modem_oper_t** opers);

/*------------------------------------------------------------------------*/

modem_network_reg_t at_network_registration(modem_t* modem);

/*------------------------------------------------------------------------*/

modem_cops_mode_t at_cops_mode(modem_t* modem);

/*------------------------------------------------------------------------*/

int at_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq);

/*------------------------------------------------------------------------*/

modem_fw_ver_t* at_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info);

/*------------------------------------------------------------------------*/

char* mc77x0_at_get_network_type(modem_t* modem, char* network, size_t len);

/*------------------------------------------------------------------------*/

char* at_get_operator_name(modem_t* modem, char* oper, size_t len);

/*------------------------------------------------------------------------*/

char* at_get_operator_number(modem_t* modem, char* oper_number, size_t len);

/*------------------------------------------------------------------------*/

int at_change_pin(modem_t* modem, const char* old_pin, const char* new_pin);

/*------------------------------------------------------------------------*/

int at_operator_select(modem_t* modem, int hni, modem_oper_act_t act);

/*------------------------------------------------------------------------*/

int at_set_wwan_profile(modem_t* modem, modem_data_profile_t* profile);

/*------------------------------------------------------------------------*/

char* at_ussd_cmd(modem_t* modem, const char* query);

#endif /* __AT_COMMON_H */

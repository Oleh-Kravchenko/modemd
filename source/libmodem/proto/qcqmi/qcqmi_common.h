#ifndef __QCQMI_COMMON_H
#define __QCQMI_COMMON_H

#include <modem/types.h>

char* qcqmi_get_imsi(modem_t* modem, char* imsi, size_t len);

char* qcqmi_get_imei(modem_t* modem, char* imei, size_t len);

time_t qcqmi_get_network_time(modem_t* modem);

int qcqmi_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq);

int qcqmi_get_cell_id(modem_t* modem);

char* qcqmi_get_network_type(modem_t* modem, char* network, size_t len);

modem_network_reg_t qcqmi_network_registration(modem_t* modem);

char* qcqmi_get_operator_name(modem_t* modem, char* oper, size_t len);

int qcqmi_operator_scan(modem_t* modem, modem_oper_t** opers);

int qcqmi_operator_select(modem_t* modem, int hni, modem_oper_act_t act);

char* qcqmi_get_operator_number(modem_t* modem, char* oper_number, size_t len);

int qcqmi_set_wwan_profile(modem_t* modem, modem_data_profile_t* profile);

int qcqmi_start_wwan(modem_t* modem);

int qcqmi_stop_wwan(modem_t* modem);

modem_state_wwan_t qcqmi_state_wwan(modem_t* modem);

modem_cpin_state_t qcqmi_cpin_state(modem_t* modem);

int qcqmi_cpin_pin(modem_t* modem, const char* pin);

int qcqmi_cpin_puk(modem_t* modem, const char* puk, const char* pin);

modem_fw_ver_t* qcqmi_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info);

#endif

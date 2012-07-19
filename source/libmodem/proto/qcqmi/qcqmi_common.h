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

#endif

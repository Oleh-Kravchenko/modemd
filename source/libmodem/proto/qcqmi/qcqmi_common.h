#ifndef __QCQMI_COMMON_H
#define __QCQMI_COMMON_H

char* qcqmi_get_imsi(modem_t* modem, char* imsi, size_t len);

char* qcqmi_get_imei(modem_t* modem, char* imei, size_t len);

time_t qcqmi_get_network_time(modem_t* modem);

#endif

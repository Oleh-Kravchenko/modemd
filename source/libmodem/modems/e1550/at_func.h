#ifndef __E1550_AT_FUNC_H
#define __E1550_AT_FUNC_H

#include <modem/types.h>

char* e1550_at_get_network_type(modem_t* modem, char* network, size_t len);

int e1550_at_get_freq_bands(modem_t* modem, freq_band_t** band_list);

int e1550_at_get_freq_band(modem_t* modem);

int e1550_at_set_freq_band(modem_t* modem, int band_index);

char* e1550_at_ussd_cmd(modem_t* modem, const char* query);

#endif /* __E1550_AT_FUNC_H */

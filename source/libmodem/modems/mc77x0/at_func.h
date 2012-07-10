#ifndef __MC77x0_AT_FUNC_H
#define __MC77x0_AT_FUNC_H

#include <modem/types.h>

unsigned int mc77x0_at_get_freq_band(modem_t* modem);

char* mc77x0_at_get_network_type(modem_t* modem, char *network, int len);

char* mc7750_at_get_network_type(modem_t* modem, char* network, int len);

int mc77x0_at_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq);

modem_network_reg_t mc77x0_at_network_registration(modem_t* modem);

void mc77x0_modem_sw_reset(modem_t* modem);

#endif /* __MC77x0_AT_FUNC_H */

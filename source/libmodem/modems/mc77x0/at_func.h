#ifndef __MC77x0_AT_FUNC_H
#define __MC77x0_AT_FUNC_H

#include <time.h>

#include <modem/types.h>

char* mc77x0_at_get_network_type(modem_t* modem, char *network, size_t len);

char* mc7750_at_get_network_type(modem_t* modem, char* network, size_t len);

int mc77x0_at_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq);

modem_network_reg_t mc77x0_at_network_registration(modem_t* modem);

void mc77x0_modem_sw_reset(modem_t* modem);

modem_fw_ver_t* mc77x0_at_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info);

int mc77x0_at_get_cell_id(modem_t* modem);

time_t mc77x0_at_get_network_time(modem_t* modem);

char* mc77x0_at_get_ccid(modem_t* modem, char* ccid, size_t len);

/* data session */

int mc77x0_at_start_wwan(modem_t* modem);

int mc77x0_at_stop_wwan(modem_t* modem);

int mc77x0_at_state_wwan(modem_t* modem);

/* band selection */

int mc77x0_at_get_freq_band(modem_t* modem);

int mc77x0_at_set_freq_band(modem_t* modem, int band_index);

int mc77x0_at_get_freq_bands(modem_t* modem, freq_band_t** band_list);


#endif /* __MC77x0_AT_FUNC_H */

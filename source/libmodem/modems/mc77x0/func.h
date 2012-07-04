#ifndef __MC77x0_FUNC_H
#define __MC77x0_FUNC_H

#include <modem/types.h>

int mc77x0_get_cell_id(modem_t* modem);

time_t mc77x0_get_network_time(modem_t* modem);

int mc77x0_operator_scan(modem_t* modem, modem_oper_t** opers);

int mc77x0_change_pin(modem_t* modem, const char* old_pin, const char* new_pin);

#endif /* __MC77x0_FUNC_H */

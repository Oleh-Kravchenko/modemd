#ifndef __MODEM_STR_H
#define __MODEM_STR_H

#include <modem/types.h>

/**
 * @brief text description for network registration
 * @param nr network registration status
 * @return pointer to const string text
 */
const char* str_network_registration(modem_network_reg_t nr);

#endif /* __MODEM_STR_H */

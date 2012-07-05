#ifndef __MODEM_STR_H
#define __MODEM_STR_H

#include <modem/types.h>

/**
 * @brief text description for network registration
 * @param nr network registration status
 * @return pointer to const string text
 */
const char* str_network_registration(modem_network_reg_t nr);

/**
 * @brief text description for protocol
 * @param proto protocol
 * @return pointer to const string text
 */
const char* str_proto(modem_proto_t proto);

#endif /* __MODEM_STR_H */

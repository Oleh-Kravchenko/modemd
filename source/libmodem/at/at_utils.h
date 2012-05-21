#ifndef __AT_UTILS_H
#define __AT_UTILS_H

#include "modem/types.h"

/*------------------------------------------------------------------------*/

/**
 * @brief parse output of AT+COPS=? command
 * @param s string in result +COPS: (...)
 * @param opers pointer to operator list
 * @return number of items, or 0 if failed
 */
int at_parse_cops_list(const char* s, modem_oper_t** opers);

/*------------------------------------------------------------------------*/

/**
 * @brief parse string and return error number
 * @param s string with ERROR or +CME:
 * @return -1 if no error
 */
int at_parse_error(const char* s);

/*------------------------------------------------------------------------*/

int mnc_get_length(const char *imsi);

#endif /* __AT_UTILS_H */

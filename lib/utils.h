#ifndef __UTILS_H
#define __UTILS_H

#include <stdint.h>

#include "modem/types.h"

/*------------------------------------------------------------------------*/

int regmatch_atoi(const char* src, const regmatch_t* re_subs);

void regmatch_ncpy(char* dst, size_t n, const char* src, const regmatch_t* re_subs);

char* regmatch_strdup(const char* src, const regmatch_t* re_subs);

/*------------------------------------------------------------------------*/

/**
 * @brief read file contents into the string buffer
 * @param filename name of file
 * @param s output buffer for string
 * @param size size of output buffer
 * @return Pointer to s, if successful
 */
char* file_get_contents(const char *filename, char* s, const int size);

/*------------------------------------------------------------------------*/

/**
 * @brief read file contents as a hex string
 * @param filename name of file
 * @return unsigned int
 */
unsigned int file_get_contents_hex(const char* filename);

/*------------------------------------------------------------------------*/

/**
 * @brief check vendor and product id on modem db
 * @param vendor vendor id
 * @param product product id
 * @return 0 if this device is not a modem
 */
int its_modem(uint16_t vendor, uint16_t product);

/*------------------------------------------------------------------------*/

/**
 * @brief open serial device
 * @param portname port name
 * @param flags flags (O_RDONLY, O_WRONLY, or O_RDWR), see man open
 * @return -1 if an error occurred
 */
int serial_open(const char* portname, int flags);

/*------------------------------------------------------------------------*/

/**
 * @brief receive tty name of AT interface of modem
 * @param port port
 * @param tty buffer for tty name of at port
 * @param tty_len buffer size
 * @return if successful pointer to tty buffer, other wise NULL
 **/
char* modem_get_at_port_name(const char* port, char* tty, int tty_len);

/*------------------------------------------------------------------------*/

/**
 * @brief parse output of AT+COPS=? command
 * @param s string in result +COPS: (...)
 * @param opers pointer to operator list
 * @return number of items, or 0 if failed
 */
int at_parse_cops_list(const char* s, modem_oper_t** opers);

/**
 * @brief parse string and return error number
 * @param s string with ERROR or +CME:
 * @return -1 if no error
 */
int at_parse_error(const char* s);

/*------------------------------------------------------------------------*/

int mnc_get_length(const char *imsi);

/*------------------------------------------------------------------------*/

char* trim_r(char* str);

#endif /* __UTILS_H */

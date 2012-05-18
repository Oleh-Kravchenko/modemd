#ifndef __SYSFS_H
#define __SYSFS_H

#include <stdint.h>
#include <dirent.h>

#include "modem/types.h"
#include "modem_db.h"

/*------------------------------------------------------------------------*/

/**
 * @brief check vendor and product id on modem db
 * @param vendor vendor id
 * @param product product id
 * @return 0 if this device is not a modem
 */
int modem_is_supported(uint16_t vendor, uint16_t product);

/*------------------------------------------------------------------------*/

const modem_db_device_t* modem_db_get_info(uint16_t vendor, uint16_t product);

/*------------------------------------------------------------------------*/

/**
 * @brief receive tty name interface protocol of modem
 * @param port port
 * @param type type of protocol
 * @param tty buffer for tty name of at port
 * @param tty_len buffer size
 * @return if successful pointer to tty buffer, otherwise NULL
 */
char* modem_get_iface_tty(const char* port, int iface, char* tty, int tty_len);

/*------------------------------------------------------------------------*/

usb_device_info_t* usb_device_get_info(const char* port, usb_device_info_t* di);

modem_find_t* modem_find_first(usb_device_info_t* mi);

modem_find_t* modem_find_next(modem_find_t* find, usb_device_info_t* mi);

void modem_find_close(modem_find_t* find);

#endif /* __SYSFS_H */

#ifndef __SYSFS_H
#define __SYSFS_H

#include <stdint.h>
#include <dirent.h>

#include <modem/types.h>

#include "modem_info.h"

/*------------------------------------------------------------------------*/

/**
 * @brief check vendor and product id on modem db
 * @param vendor name of manufacturer
 * @param product name of device
 * @param vendor_id vendor id
 * @param product_id product id
 * @return 0 if this device is not a modem
 */
int modem_is_supported(const char* vendor, const char* product, uint16_t vendor_id, uint16_t product_id);

/*------------------------------------------------------------------------*/

const modem_db_device_t* modem_db_get_info(const char* vendor, const char* product, uint16_t vendor_id, uint16_t product_id);

/*------------------------------------------------------------------------*/

/**
 * @brief receive tty name interface protocol of modem
 * @param port port
 * @param dev_type ttyUSB, qcqmi, etc
 * @param type type of protocol
 * @param tty buffer for tty name of at port
 * @param tty_len buffer size
 * @return if successful pointer to tty buffer, otherwise NULL
 */
char* modem_get_iface_dev(const char* port, const char* dev_type, int iface, char* dev, int dev_len);

/*------------------------------------------------------------------------*/

usb_device_info_t* usb_device_get_info(const char* port, usb_device_info_t* di);

modem_find_t* modem_find_first(usb_device_info_t* mi);

modem_find_t* modem_find_next(modem_find_t* find, usb_device_info_t* mi);

void modem_find_close(modem_find_t* find);

#endif /* __SYSFS_H */

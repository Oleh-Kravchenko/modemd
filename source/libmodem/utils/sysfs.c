#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>

#include "modem/types.h"

#include "file.h"
#include "re.h"
#include "modem_db.h"

/*------------------------------------------------------------------------*/

int modem_is_supported(uint16_t vendor, uint16_t product)
{
	int i;

    for(i = 0; i < modem_db_devices_cnt; ++ i)
        if
        (
			modem_db_devices[i].vendor == vendor &&
			modem_db_devices[i].product == product
		)
			return(1);

    return(0);
}

/*------------------------------------------------------------------------*/

const modem_db_device_t* modem_db_get_info(uint16_t vendor, uint16_t product)
{
	int i;

	for(i = 0; i < modem_db_devices_cnt; ++ i)
	{
		if
		(
			modem_db_devices[i].vendor == vendor &&
			modem_db_devices[i].product == product
		)
			return(&modem_db_devices[i]);
	}

	return(NULL);
}

/*------------------------------------------------------------------------*/

char* modem_get_iface_tty(const char* port, int iface, char* tty, int tty_len)
{
    char *s, *res = NULL;
    struct dirent *item;
    char path[0x100];
    DIR *dir;
    int i, j;

	snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s:1.%d/", port, iface);

    if((dir = opendir(path)) == NULL)
		return(res);

    /* getting tty name */
    while((item = readdir(dir)))
    {
        if((s = strstr(item->d_name, "ttyUSB")) == item->d_name)
        {
            /* name of tty in /dev */
            snprintf(tty, tty_len - 1, "/dev/%s", s);

            res = tty;
            break;
        }
    }

    closedir(dir);

    return(res);
}

/*------------------------------------------------------------------------*/

usb_device_info_t* usb_device_get_info(const char* port, usb_device_info_t* di)
{
    char path[0x100];

    /* read device name and id */
    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", port);
    if(!(di->id_vendor = file_get_contents_hex(path)))
        return(NULL);

    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", port);
    if(!(di->id_product = file_get_contents_hex(path)))
        return(NULL);

    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/manufacturer", port);
    if(!file_get_contents(path, di->manufacturer, sizeof(di->manufacturer)))
        return(NULL);

    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/product", port);
    if(!file_get_contents(path, di->product, sizeof(di->product)))
        return(NULL);

    strncpy(di->port, port, sizeof(di->port) - 1);
    di->port[sizeof(di->port) - 1] = 0;

    return(di);
}

/*------------------------------------------------------------------------*/

modem_find_t* modem_find_first(usb_device_info_t* mi)
{
    struct dirent *sysfs_item;
    DIR *res;

    if(!(res = opendir("/sys/bus/usb/devices/")))
        goto failed_open;

    while((sysfs_item = readdir(res)))
    {
        /* directory name must be in format BUS-DEV */
        if(regmatch_cmp(sysfs_item->d_name, "^[0-9]-[0-9]$") != 0)
            continue;

        if(!(usb_device_get_info(sysfs_item->d_name, mi)))
            goto err;

        /* check device on modem db */
        if(!modem_is_supported(mi->id_vendor, mi->id_product))
            continue;

        return(res);
    }

err:
    closedir(res);

	res = NULL;

failed_open:
    return(res);
}

/*------------------------------------------------------------------------*/

modem_find_t* modem_find_next(modem_find_t* find, usb_device_info_t* mi)
{
    struct dirent *sysfs_item;

    while((sysfs_item = readdir(find)))
    {
        /* directory name must be in format BUS-DEV */
        if(regmatch_cmp(sysfs_item->d_name, "^[0-9]-[0-9]$") != 0)
            continue;

        if(!usb_device_get_info(sysfs_item->d_name, mi))
            goto err;

        /* check device on modem db */
        if(!modem_is_supported(mi->id_vendor, mi->id_product))
            continue;

        return(find);
    }

    closedir(find);

err:
    return(NULL);
}

/*------------------------------------------------------------------------*/

void modem_find_close(modem_find_t* find)
{
	if(find)
		closedir(find);
}

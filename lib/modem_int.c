#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <regex.h>

#include "modem/types.h"

#include "utils.h"
#include "rpc.h"

/*------------------------------------------------------------------------*/

modem_info_t* usb_device_info(const char* port)
{
    modem_info_t* res;
    char path[0x100];

    if(!(res = malloc(sizeof(*res))))
        goto err;

    /* read device name and id */
    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", port);
    res->id_vendor = file_get_contents_hex(path);

    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", port);
    res->id_product = file_get_contents_hex(path);

    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/manufacturer", port);
    file_get_contents(path, res->manufacturer, sizeof(res->manufacturer));

    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/product", port);
    file_get_contents(path, res->product, sizeof(res->product));

    strncpy(res->port, port, sizeof(res->port) - 1);
    res->port[sizeof(res->port) - 1] = 0;

err:
    return(res);
}

/*------------------------------------------------------------------------*/

modem_info_t* modem_find_first(DIR **dir)
{
    modem_info_t* res;
    DIR *sysfs_dir;
    struct dirent *sysfs_item;
    regex_t reg;
    int reg_res;

    if(!(sysfs_dir = opendir("/sys/bus/usb/devices/")))
        goto failed_open;

    while((sysfs_item = readdir(sysfs_dir)))
    {
        /* directory name must be in format BUS-DEV */
        regcomp(&reg, "^[0-9]-[0-9]$", 0);
        reg_res = regexec(&reg, sysfs_item->d_name, 0, NULL, 0);
        regfree(&reg);

        if(reg_res != 0)
            continue;

        if(!(res = usb_device_info(sysfs_item->d_name)))
            goto err;

        /* check device on modem db */
        if(!its_modem(res->id_vendor, res->id_product))
        {
            free(res);
            res = NULL;
            continue;
        }

        *dir = sysfs_dir;

        return(res);
    }

err:
    closedir(sysfs_dir);

failed_open:
    return(NULL);
}

/*------------------------------------------------------------------------*/

modem_info_t* modem_find_next(DIR **dir)
{
    struct dirent *sysfs_item;
    modem_info_t* res = NULL;
    regex_t reg;
    int reg_res;

    while((sysfs_item = readdir(*dir)))
    {
        /* name if directory must be in BUS-DEV format */
        regcomp(&reg, "^[0-9]-[0-9]$", 0);
        reg_res = regexec(&reg, sysfs_item->d_name, 0, NULL, 0);
        regfree(&reg);

        if(reg_res != 0)
            continue;

        if(!(res = usb_device_info(sysfs_item->d_name)))
            goto err;

        /* check device on modem db */
        if(!its_modem(res->id_vendor, res->id_product))
        {
            free(res);
            res = NULL;
            continue;
        }

        return(res);
    }

    closedir(*dir);
    *dir = NULL;
    free(res);

err:
    return(NULL);
}

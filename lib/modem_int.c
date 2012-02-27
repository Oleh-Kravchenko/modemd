#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <regex.h>

#include "modem/types.h"

/*-------------------------------------------------------------------------*/

modem_info_t* modem_find_first(DIR **dir)
{
    modem_info_t* res;
    DIR *sysfs_dir;
    struct dirent *sysfs_item;
    regex_t reg;
    int reg_res;
    char path[256];

    if(!(sysfs_dir = opendir("/sys/bus/usb/devices/")))
        goto failed_open;

    while((sysfs_item = readdir(sysfs_dir)))
    {
        // имя каталога должно быть в таком формате BUS-DEV
        regcomp(&reg, "^[0-9]-[0-9]$", 0);
        reg_res = regexec(&reg, sysfs_item->d_name, 0, NULL, 0);
        regfree(&reg);

        if(reg_res != 0)
            continue;

        if(!(res = malloc(sizeof(*res))))
            goto exit;

        // читаем индфикаторы и имя ус-ва
        strncpy(res->port, sysfs_item->d_name, sizeof(res->port) - 1);

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/manufacturer", sysfs_item->d_name);
        file_get_contents(path, res->manufacturer, sizeof(res->manufacturer));

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/product", sysfs_item->d_name);
        file_get_contents(path, res->product, sizeof(res->product));

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", sysfs_item->d_name);
        res->id_vendor = file_get_contents_hex(path);

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", sysfs_item->d_name);
        res->id_product = file_get_contents_hex(path);

        *dir = sysfs_dir;

        return(res);
    }

exit:
    closedir(sysfs_dir);

failed_open:
    return(NULL);
}

/*-------------------------------------------------------------------------*/

modem_info_t* modem_find_next(DIR **dir)
{
    struct dirent *sysfs_item;
    modem_info_t* res = NULL;
    regex_t reg;
    int reg_res;
    char path[256];

    while((sysfs_item = readdir(*dir)))
    {
        // имя каталога должно быть в таком формате BUS-DEV
        regcomp(&reg, "^[0-9]-[0-9]$", 0);
        reg_res = regexec(&reg, sysfs_item->d_name, 0, NULL, 0);
        regfree(&reg);

        if(reg_res != 0)
            continue;

        if(!(res = malloc(sizeof(*res))))
            goto exit;

        // читаем индфикаторы и имя ус-ва
        strncpy(res->port, sysfs_item->d_name, sizeof(res->port) - 1);

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/manufacturer", sysfs_item->d_name);
        file_get_contents(path, res->manufacturer, sizeof(res->manufacturer));

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/product", sysfs_item->d_name);
        file_get_contents(path, res->product, sizeof(res->product));

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", sysfs_item->d_name);
        res->id_vendor = file_get_contents_hex(path);

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", sysfs_item->d_name);
        res->id_product = file_get_contents_hex(path);

        return(res);
    }

    closedir(*dir);
    *dir = NULL;
    free(res);

exit:
    return(NULL);
}

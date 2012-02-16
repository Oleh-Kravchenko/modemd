#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>

#include <libgen.h>

#include "../modem.h"

#include <stdio.h>
#include <dirent.h>
#include <string.h>

#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

/*-------------------------------------------------------------------------*/

char* file_get_contents(const char *filename, char* s, const int size)
{
    FILE *f;
    char* res;

    if(!(f = fopen(filename, "r")))
        return(0);

    res = fgets(s, size, f);

    fclose(f);

    int rn = strlen(res);

    /* removing eof */
    if(rn && (res[rn - 1] == '\n' || res[rn - 1] == '\n'))
        res[rn - 1] = 0;

    return(res);
}

/*-------------------------------------------------------------------------*/

int file_get_contents_hex(const char* filename)
{
    char hex[256];
    int res = 0;

    if(file_get_contents(filename, hex, sizeof(hex)))
        sscanf(hex, "%x", &res);

    return(res);
}

/*-------------------------------------------------------------------------*/

modem_info_t* modem_find_first(void)
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

        if(!(res = malloc(sizeof(modem_info_t))))
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

        res->sysfs_dir = sysfs_dir;

        return(res);
    }

exit:
    closedir(sysfs_dir);

failed_open:
    return(NULL);
}

/*-------------------------------------------------------------------------*/

modem_info_t* modem_find_next(modem_info_t* modem)
{
    struct dirent *sysfs_item;
    regex_t reg;
    int reg_res;
    char path[256];

    while((sysfs_item = readdir(modem->sysfs_dir)))
    {
        // имя каталога должно быть в таком формате BUS-DEV
        regcomp(&reg, "^[0-9]-[0-9]$", 0);
        reg_res = regexec(&reg, sysfs_item->d_name, 0, NULL, 0);
        regfree(&reg);

        if(reg_res != 0)
            continue;

        // читаем индфикаторы и имя ус-ва
        strncpy(modem->port, sysfs_item->d_name, sizeof(modem->port) - 1);

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/manufacturer", sysfs_item->d_name);
        file_get_contents(path, modem->manufacturer, sizeof(modem->manufacturer));

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/product", sysfs_item->d_name);
        file_get_contents(path, modem->product, sizeof(modem->product));

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", sysfs_item->d_name);
        modem->id_vendor = file_get_contents_hex(path);

        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", sysfs_item->d_name);
        modem->id_product = file_get_contents_hex(path);

        return(modem);
    }

    closedir(modem->sysfs_dir);
    free(modem);

    return(NULL);
}

/*-------------------------------------------------------------------------*/

/*    modem_info_t *modem;

    modem = modem_find_first();

    while(modem)
    {
        printf("%s %04hx:%04hx %s %s\n", modem->port, modem->id_vendor, modem->id_product, modem->manufacturer, modem->product);

        modem = modem_find_next(modem);
    }
*/

/*-------------------------------------------------------------------------*/

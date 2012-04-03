#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <regex.h>

#include "utils.h"

/*------------------------------------------------------------------------*/

char* file_get_contents(const char *filename, char* s, const int size)
{
    char* res;
    FILE *f;
    int rn;

    if(!(f = fopen(filename, "r")))
        return(0);

    res = fgets(s, size, f);

    fclose(f);

    rn = strlen(res);

    /* removing eof */
    if(rn && (res[rn - 1] == '\n' || res[rn - 1] == '\n'))
        res[rn - 1] = 0;

    return(res);
}

/*------------------------------------------------------------------------*/

unsigned int file_get_contents_hex(const char* filename)
{
    char hex[256];
    unsigned int res = 0;

    if(file_get_contents(filename, hex, sizeof(hex)))
        sscanf(hex, "%x", &res);

    return(res);
}

/*------------------------------------------------------------------------*/

int its_modem(uint16_t vendor, uint16_t product)
{
    int res;

    res = vendor == 0x1199 && product == 0x68a3;
    res += vendor == 0x1199 && product == 0x68a2;
    res += vendor == 0x12d1 && product == 0x1001;

    return(res);
}

/*------------------------------------------------------------------------*/

int serial_open(const char* portname, int flags)
{
    struct termios tp;
    int port;

    port = open(portname, flags);

    if(port < 0)
        return(-1);

    /* serial port settings */
    tcgetattr(port, &tp);

    /* make sure tp is all blank and no residual values set
    safer than modifying current settings */
    tp.c_lflag = 0; /* implies non-canoical mode */
    tp.c_iflag = 0;
    tp.c_oflag = 0;
    tp.c_cflag = 0;
    tp.c_cflag |= B115200;
    tp.c_cflag |= CS8;

    /* ignore modem lines like hangup */
    tp.c_cflag |= CLOCAL;

    /* let us read from this device! */
    tp.c_cflag |= CREAD;

    /* perform newline mapping, useful when dealing with dos/windows systems */
    tp.c_oflag = OPOST;

    /* pay attention to hangup line */
    tp.c_cflag |= HUPCL;

    /* don't wait between receiving characters */
    tp.c_cc[VMIN] = 1;
    tp.c_cc[VTIME] = 0;

    tcsetattr(port, TCSANOW, &tp);

    return(port);
}

/*------------------------------------------------------------------------*/

char* modem_get_at_port_name(const char* port, char* tty, int tty_len)
{
    uint16_t vendor, product;
    char* at_port, *res = NULL;
    struct dirent *item;
    char path[0xff];
    DIR *dir;

    /* reading id's of modem */
    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", port);
    vendor = file_get_contents_hex(path);

    snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", port);
    product = file_get_contents_hex(path);

    /* getting interface number */
    if((vendor == 0x1199 && product == 0x68a2))
        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s:1.3/", port);
    else if((vendor == 0x1199 && product == 0x68a3))
        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s:1.3/", port);
    else if((vendor == 0x12d1 && product == 0x1001))
        snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s:1.0/", port);
    else
        goto err;

    if(!(dir = opendir(path)))
        goto err;

    /* getting tty name */
    while((item = readdir(dir)))
    {
        if((at_port = strstr(item->d_name, "ttyUSB")))
        {
            snprintf(tty, tty_len - 1, "/dev/%s", at_port);

            res = tty;
            break;
        }
    }

    closedir(dir);

err:
    return(res);
}

/*------------------------------------------------------------------------*/

int at_parse_cops_list(const char* s, modem_oper_t** opers)
{
    regmatch_t *re_subs;
    int len = strlen(s);
    int next = 0;
    int nopers;
    regex_t re;

    nopers = 0;
    *opers = NULL;

    if(regcomp(&re, "\\(([0123]),\"(.{0,16})\",\"(.{0,16})\",\"([0-9]{5,16})\",([012])),?", REG_EXTENDED))
        goto err;

    /* memory for regexp result */
    re_subs = malloc(sizeof(regmatch_t) * (re.re_nsub + 1));

    if(!re_subs)
        goto malloc_err;

    while(next < len)
    {
        if(regexec(&re, s + next, (re.re_nsub + 1), re_subs, 0))
            break;

        ++ nopers;

        /* increase memory for result */
        *opers = realloc(*opers, sizeof(modem_oper_t) * nopers);

        /* add new info about operator */
        (*opers)[nopers - 1].stat = (*(s + next + re_subs[1].rm_so)) - 0x30;
        __REGMATCH_CUT((*opers)[nopers - 1].longname, s + next, re_subs[2]);
        __REGMATCH_CUT((*opers)[nopers - 1].shortname, s + next, re_subs[3]);
        __REGMATCH_CUT((*opers)[nopers - 1].numeric, s + next, re_subs[4]);
        (*opers)[nopers - 1].act = (*(s + next + re_subs[5].rm_so)) - 0x30;

        /* length of one operator string */
        next += re_subs[0].rm_eo - re_subs[0].rm_so;
    }

    free(re_subs);

malloc_err:
    regfree(&re);

err:
    return(nopers);
}

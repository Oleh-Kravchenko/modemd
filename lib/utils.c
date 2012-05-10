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
    modem_oper_t* oper;
    const char *t = s;
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
        if(regexec(&re, t, (re.re_nsub + 1), re_subs, 0))
            break;

        /* increase memory for result */
        *opers = realloc(*opers, sizeof(modem_oper_t) * (nopers + 1));

        oper = (*opers + nopers);

        /* add new info about operator */
        oper->stat = regmatch_atoi(t, re_subs + 1);
        regmatch_ncpy(oper->longname, sizeof(oper->longname), t, re_subs + 2);
        regmatch_ncpy(oper->shortname, sizeof(oper->shortname), t, re_subs + 3);
        regmatch_ncpy(oper->numeric, sizeof(oper->numeric), t, re_subs + 4);
        oper->act = regmatch_atoi(t, re_subs + 5);

        ++ nopers;

        /* length of one operator string */
        next += re_subs[0].rm_eo - re_subs[0].rm_so;
        t = s + next;
    }

    free(re_subs);

malloc_err:
    regfree(&re);

err:
    return(nopers);
}

/*------------------------------------------------------------------------*/

int at_parse_error(const char* s)
{
    regmatch_t *re_subs;
    int cme_error = -1;
    char serror[6];
    regex_t re;

    if(strstr(s, "\r\nERROR\r\n"))
        /* modem failure (general error) or reporting is AT+CMEE=0 */
        return(0);

    if(regcomp(&re, "\\+CME ERROR: ([0-9]{1,5})\r\n", REG_EXTENDED))
        goto err;

    /* memory for regexp result */
    re_subs = malloc(sizeof(regmatch_t) * (re.re_nsub + 1));

    if(!re_subs)
        goto malloc_err;

    if(regexec(&re, s, (re.re_nsub + 1), re_subs, 0))
        goto reg_err;

    regmatch_ncpy(serror, sizeof(serror), s, re_subs + 1);

    /* CME error */
    cme_error = atoi(serror);

reg_err:
    free(re_subs);

malloc_err:
    regfree(&re);

err:
    return(cme_error);
}

/*------------------------------------------------------------------------*/

int mnc_get_length(const char *imsi)
{
#define MCC_LEN 3
#define MAX_MNC_LEN 3

	int nspec_mcc[] = {302, 310, 311, 316, 334, 338, 342, 344, 346, 348, 365, 374, 376, 708, 722, 732, 0};
	char mcc[MCC_LEN + 1];
	int i = 0, found = 0, ret;
	int nmcc;
	// If MCC belongs to special MCC group, check if it's a "super special MCC" :-)
	char mnc[MAX_MNC_LEN + 1];

	//! IMSI value (15 digits), starting with MCC (3 digits) / MNC (2 digits, 3 for PCS 1900)
	//! Mobile operators with MCC listed in 'iSpecialMCC' has 3 digits in MNC
	//! Mobile operators with MCC 302 or 374 may have MNC with 2 or 3 digits
	//! For existing MCCs and MNCs see the "MOBILE NETWORK CODES (MNC) FOR THE INTERNATIONAL
	//! IDENTIFICATION PLAN FOR PUBLIC NETWORKS AND SUBSCRIPTIONS" document
	//! of TELECOMMUNICATION STANDARDIZATION BUREAU OF International Telecommunication Union


	//! Extract MCC
	strncpy( mcc, imsi, MCC_LEN );
	mcc[MCC_LEN] = 0;

	nmcc = atoi(mcc);

	//! Check if MCC belogs to special MCC group (3 digits MNC)
	while(nspec_mcc[i] && !found)
	{
		if(nmcc == nspec_mcc[i])
		{
			found = 1;
			break;
		}

		++ i;
	}

	if(!found)
	{
		ret = 2;
		goto exit;
	}

	switch(nmcc)
	{
		case 302:
			strncpy(mnc, imsi + MCC_LEN, 3 );
			mnc[MCC_LEN] = 0;

			ret = (atoi(mnc) == 656 ? 2 : 3);
			break;

		case 374:
			strncpy( mnc, imsi + MCC_LEN, 3 );
			mnc[MCC_LEN] = 0;

			ret = (atoi(mnc) == 12 ? 2 : 3);
			break;

		default:
			ret = 3;
	}

exit:
	return ret;
}

/*------------------------------------------------------------------------*/

static const char escape_symbols[] = "\x20\a\b\t\n\v\f\r";

/*------------------------------------------------------------------------*/

char* trim_l(char* s)
{
	char* res = s;

	while(*s && strchr(escape_symbols, *s))
		++ s;

	if(res != s)
		memmove(res, s, strlen(s) + 1);

	return(res);
}

/*------------------------------------------------------------------------*/

char* trim_r(char* s)
{
	size_t len = strlen(s);
	char *res = NULL;

	while(len -- && strchr(escape_symbols, s[len]))
		res = &s[len];

	if(res)
		*res = 0;

	return(res);
}

/*------------------------------------------------------------------------*/

char* trim(char *s)
{
	char* l = s;
	size_t len;

	while(*l && strchr(escape_symbols, *l))
		++ l;

	len = strlen(l);

	while(len)
	{
		-- len;

		if(!strchr(escape_symbols, l[len]))
			break;
	}

	if(len ++)
	{
		memmove(s, l, len);
		s[len] = 0;
	}

	return(s + len);
}

/*------------------------------------------------------------------------*/

int regmatch_atoi(const char* src, const regmatch_t* re_subs)
{
    int res = 0;
    size_t len;
    char* buf;

    len = re_subs->rm_eo - re_subs->rm_so;

    if(!(buf = malloc(len + 1)))
        return(res);

    memcpy(buf, src + re_subs->rm_so, len);
    buf[len] = 0;

    res = atoi(buf);

    free(buf);

    return(res);
}

/*------------------------------------------------------------------------*/

void regmatch_ncpy(char* dst, size_t n, const char* src, const regmatch_t* re_subs)
{
    size_t len;

    len = re_subs->rm_eo - re_subs->rm_so;
    len = (len > n ? n - 1 : len);

    memcpy(dst, src + re_subs->rm_so, len);
    dst[len] = 0;
}

/*------------------------------------------------------------------------*/

char* regmatch_strdup(const char* src, const regmatch_t* re_subs)
{
    size_t len;
    char* res;

    len = re_subs->rm_eo - re_subs->rm_so;

    if(!(res = malloc(len + 1)))
        return(res);

    memcpy(res, src + re_subs->rm_so, len);
    res[len] = 0;

    return(res);
}

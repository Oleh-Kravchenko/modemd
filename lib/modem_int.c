#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <regex.h>

#include "modem/types.h"

#include "utils.h"
#include "rpc.h"

#include "../queue.h"
#include "../mc7700.h"

/*------------------------------------------------------------------------*/

char *strptime(const char *s, const char *format, struct tm *tm);

/*------------------------------------------------------------------------*/

modem_info_t* usb_device_get_info(const char* port)
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

        if(!(res = usb_device_get_info(sysfs_item->d_name)))
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

        if(!(res = usb_device_get_info(sysfs_item->d_name)))
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

/*------------------------------------------------------------------------*/

modem_cpin_state_t at_cpin_state(queue_t* queue)
{
    modem_cpin_state_t res = MODEM_CPIN_STATE_UNKNOWN;
    mc7700_query_t* q;
    char cpin_st[8];

    q = mc7700_query_create("AT+CPIN?\r\n", "\r\n\\+CPIN: ((READY)|(SIM PIN)|(SIM PUK))\r\n\r\nOK\r\n");
    mc7700_query_execute(queue, q);

    if(q->answer)
    {
        __REGMATCH_N_CUT(cpin_st, sizeof(cpin_st), q->answer, q->re_subs[1])

        if(strcmp(cpin_st, "READY") == 0)
            res = MODEM_CPIN_STATE_READY;
        else if(strcmp(cpin_st, "SIM PIN") == 0)
            res = MODEM_CPIN_STATE_PIN;
        else if(strcmp(cpin_st, "SIM PUK") == 0)
            res = MODEM_CPIN_STATE_PUK;
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

int at_cpin_pin(queue_t* queue, const char* pin)
{
    mc7700_query_t* q;
    char cmd[32];
    int res;

    snprintf(cmd, sizeof(cmd), "AT+CPIN=\"%s\"\r\n", pin);
    q = mc7700_query_create(cmd, "\r\nOK\r\n");
    mc7700_query_execute(queue, q);

    res = (q->answer ? 0 : -1);

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

int at_cpin_puk(queue_t* queue, const char* puk, const char* pin)
{
    mc7700_query_t* q;
    char cmd[0x100];
    int res;

    snprintf(cmd, sizeof(cmd), "AT+CPIN=\"%s\",\"%s\"\r\n", puk, pin);
    q = mc7700_query_create(cmd, "\r\nOK\r\n");
    mc7700_query_execute(queue, q);

    res = (q->answer ? 0 : -1);

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

int at_raw_ok(queue_t* queue, const char* cmd)
{
    mc7700_query_t* q;
    int res;

    q = mc7700_query_create(cmd, "\r\nOK\r\n");
    q->timeout = 10;
    mc7700_query_execute(queue, q);

    res = (q->answer ? 0 : -1);

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

char* at_get_imsi(queue_t* queue, char* imsi, size_t len)
{
    mc7700_query_t *q;
    char *res = NULL;

    q = mc7700_query_create("AT+CIMI\r\n", "\r\n([0-9]+)\r\n\r\nOK\r\n");
    mc7700_query_execute(mc7700_thread_priv.q, q);

    /* cutting IMSI number from the reply */
    if(q->answer)
    {
        __REGMATCH_N_CUT(imsi, len, q->answer, q->re_subs[1])

        res = imsi;
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

char* at_get_imei(queue_t* queue, char* imei, size_t len)
{
    mc7700_query_t *q;
    char *res = NULL;

    q = mc7700_query_create("AT+CGSN\r\n", "\r\n([0-9]+)\r\n\r\nOK\r\n");
    mc7700_query_execute(mc7700_thread_priv.q, q);

    if(q->answer)
    {
        __REGMATCH_N_CUT(imei, len, q->answer, q->re_subs[1])

        res = imei;
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

int at_operator_scan(queue_t* queue, modem_oper_t** opers)
{
    mc7700_query_t *q;
    int nopers = 0;
    char *sopers;

    q = mc7700_query_create("AT+COPS=?\r\n", "\r\n\\+COPS: (.+),,\\(.+\\),\\(.+\\)\r\n\r\nOK\r\n");
    q->timeout = 120;

    mc7700_query_execute(queue, q);

    /* cutting operators from the answer */
    if(!q->answer)
        goto exit;

    if(!(sopers = malloc(q->re_subs[1].rm_eo - q->re_subs[1].rm_so + 1)))
        goto err_malloc;

    /* parsing operator list */
    __REGMATCH_CUT(sopers, q->answer, q->re_subs[1]);

    nopers = at_parse_cops_list(sopers, opers);

err_malloc:
    free(sopers);

exit:
    mc7700_query_destroy(q);

    return(nopers);
}

/*------------------------------------------------------------------------*/

void* at_thread_operator_scan(void* prm)
{
    at_operator_scan_t *priv = prm;
    modem_oper_t* opers;
    int nopers, i;
    FILE *f;

    if((nopers = at_operator_scan(priv->priv->q, &opers)) == 0)
        goto exit;

    if(!(f = fopen(priv->file, "w")))
        goto err_fopen;

    for(i = 0; i < nopers; ++ i)
        fprintf(f, "%s,%s,%d\n", opers[i].numeric,
            strlen(opers[i].shortname) ?
            opers[i].shortname : opers[i].numeric, opers[i].act);

    fclose(f);

err_fopen:
    free(opers);

exit:
    free(priv);
    return(NULL);
}

/*------------------------------------------------------------------------*/

modem_network_reg_t at_network_registration(queue_t* queue)
{
    modem_network_reg_t nr = MODEM_NETWORK_REG_UNKNOWN;
    mc7700_query_t *q;
    int nnr;

#ifdef __HW_C1KMBR
    q = mc7700_query_create("AT+CEREG?\r\n", "\r\n\\+CEREG: [0-9],([0-9])\r\n\r\nOK\r\n");
#else
    q = mc7700_query_create("AT+CREG?\r\n", "\r\n\\+CREG: [0-9],([0-9])\r\n\r\nOK\r\n");
#endif /* __HW_C1KMBR */

    mc7700_query_execute(queue, q);

    if(q->answer)
    {
        /* cutting registration status from the reply and check value */
        /* fast ASCII digit conversion (char - 0x30) */
        nnr = *(q->answer + q->re_subs[1].rm_so) - 0x30;
        nr = (nnr >= 0 && nnr <= 5) ? nnr : MODEM_NETWORK_REG_UNKNOWN;
    }

    mc7700_query_destroy(q);

    return(nr);
}

/*------------------------------------------------------------------------*/

modem_cops_mode_t at_cops_mode(queue_t* queue)
{
    modem_cops_mode_t nr = MODEM_COPS_MODE_UNKNOWN;
    mc7700_query_t *q;
    int nnr;

    q = mc7700_query_create("AT+COPS?\r\n", "\\+COPS: ([01234]),?.*\r\n\r\nOK\r\n");

    mc7700_query_execute(queue, q);

    if(q->answer)
    {
        /* cutting registration status from the reply and check value */
        /* fast ASCII digit conversion (char - 0x30) */
        nnr = *(q->answer + q->re_subs[1].rm_so) - 0x30;
        nr = (nnr >= 0 && nnr <= 4) ? nnr : MODEM_COPS_MODE_UNKNOWN;
    }

    mc7700_query_destroy(q);

    return(nr);
}

/*------------------------------------------------------------------------*/

int at_get_signal_quality(queue_t *queue, modem_signal_quality_t* sq)
{
    char rssi[16], ber[16];
    mc7700_query_t *q;
    int res = -1, nrssi, nber;

    sq->dbm = 0;
    sq->level = 0;

    q = mc7700_query_create("AT+CSQ\r\n", "([0-9]+), ?([0-9]+)\r\n\r\nOK\r\n");
    mc7700_query_execute(queue, q);

    /* cutting IMEI number from the reply */
    if(q->answer)
    {
        __REGMATCH_N_CUT(rssi, sizeof(rssi), q->answer, q->re_subs[1])
        __REGMATCH_N_CUT(ber, sizeof(ber), q->answer, q->re_subs[2])

        nrssi = atoi(rssi);
        nber = atoi(ber);

        /* calculation dBm */
        sq->dbm = (nrssi > 31) ? 0 : nrssi * 2 - 113;

        /* calculation signal level */
        if((sq->level = !!sq->dbm))
        {
            sq->level += (sq->dbm >= -95);
            sq->level += (sq->dbm >= -85);
            sq->level += (sq->dbm >= -73);
            sq->level += (sq->dbm >= -65);
        }
        
        res = 0;
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

modem_fw_version_t* at_get_fw_version(queue_t *queue, modem_fw_version_t* fw_info)
{
    modem_fw_version_t* res = NULL;
    mc7700_query_t *q;
    char firmware[0x100];
    char release[0x100];
    struct tm tm;

    q = mc7700_query_create("AT+CGMR\r\n", "\r\n.*(SWI.*) .* .* ([0-9,/]+ [0-9,:]+)\r\n\r\nOK\r\n");

    mc7700_query_execute(queue, q);

    /* cutting Operator name from the answer */
    if(q->answer)
    {
        __REGMATCH_N_CUT(firmware, sizeof(firmware), q->answer, q->re_subs[1])
        __REGMATCH_N_CUT(release, sizeof(release), q->answer, q->re_subs[2])

        /* create result */
        strncpy(fw_info->firmware, firmware, sizeof(fw_info->firmware) - 1);
        fw_info->firmware[sizeof(fw_info->firmware) - 1] = 0;

        /* parsing date and time */
        strptime(release, "%Y/%m/%d\r\n%H:%M:%S", &tm);
        fw_info->release = mktime(&tm);

        res = fw_info;
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

char* at_get_network_type(queue_t *queue, char *network, int len)
{
    mc7700_query_t *q;
    char *res = NULL;

    q = mc7700_query_create("AT*CNTI=0\r\n", "\r\n\\*CNTI: 0,(.+)\r\n\r\nOK\r\n");

    mc7700_query_execute(queue, q);

    if(q->answer)
    {
        __REGMATCH_N_CUT(network, len, q->answer, q->re_subs[1])

        res = network;
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

char* at_get_operator_name(queue_t *queue, char *oper, int len)
{
    mc7700_query_t *q;
    int res_ok;
    char *res = NULL;

    /* setup format of +COPS as a string */
    q = mc7700_query_create("AT+COPS=3,0\r\n", "\r\nOK\r\n");
    mc7700_query_execute(mc7700_thread_priv.q, q);
    res_ok = !!q->answer;
    mc7700_query_destroy(q);

    if(!res_ok)
        return(res);

    q = mc7700_query_create("AT+COPS?\r\n", "\r\n\\+COPS: [0-9],[0-9],\"(.+)\",[0-9]\r\n\r\nOK\r\n");
    mc7700_query_execute(mc7700_thread_priv.q, q);

    /* cutting Operator name from the answer */
    if(q->answer)
    {
        __REGMATCH_N_CUT(oper, len, q->answer, q->re_subs[1])

        res = oper;
    }

    mc7700_query_destroy(q);

    return(res);
}

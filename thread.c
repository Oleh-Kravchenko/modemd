#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include "thread.h"
#include "mc7700.h"
#include "modem/types.h"
#include "lib/rpc.h"
#include "lib/modem_int.h"
#include "lib/utils.h"
#include "hardware.h"

/*------------------------------------------------------------------------*/

char *strptime(const char *s, const char *format, struct tm *tm);

/*------------------------------------------------------------------------*/

typedef rpc_packet_t* (*rpc_function_t)(modem_client_thread_t*, rpc_packet_t*);

/*------------------------------------------------------------------------*/

typedef struct
{
    char name[0x100];

    rpc_function_t func;
} rpc_function_info_t;

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_find_first_packet(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    modem_info_t *mi;
    int mi_len = sizeof(*mi);

    if(!(mi = modem_find_first(&priv->dir)))
        mi_len = 0;

    res = rpc_create(TYPE_RESPONSE, "modem_find_first", (uint8_t*)mi, mi_len);

    free(mi);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_find_next_packet(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    modem_info_t *mi;
    int mi_len = sizeof(*mi);

    if(!(mi = modem_find_next(&priv->dir)))
        mi_len = 0;

    res = rpc_create(TYPE_RESPONSE, "modem_find_next", (uint8_t*)mi, mi_len);

    free(mi);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_open_by_port(modem_client_thread_t* priv, rpc_packet_t* p)
{
    char port[0xff] = {"A"};
    char tty[0xff] = {0};
    int path_len, modem = 0;
    rpc_packet_t *res = NULL;
    int modem_wakeup_tries;

    path_len = p->hdr.data_len > sizeof(port) ? sizeof(port) : p->hdr.data_len;
    memcpy(port, p->data, path_len);
    port[path_len] = 0;

    modem_wakeup_tries = 3;

    while(!modem_get_at_port_name(port, tty, sizeof(tty)) && modem_wakeup_tries)
    {
#ifdef __MODEMD_DEBUG
        printf("(II) Wait modem wakeup on port %s\n", port);
#endif
        init_port(port);
        sleep(20);
        -- modem_wakeup_tries;
    }

    if(*tty)
    {
#ifdef __MODEMD_DEBUG
        printf("==== %s -> %s\n", port, tty);
#endif
        strncpy(priv->port, port, sizeof(priv->port) - 1);
        priv->port[sizeof(priv->port) - 1] = 0;

        modem = mc7700_open(tty);
    }

    res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&modem, sizeof(modem));

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_close(modem_client_thread_t* priv, rpc_packet_t* p)
{
    mc7700_destroy();

    return(rpc_create(TYPE_RESPONSE, __func__, NULL, 0));
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_imei(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;

    q = mc7700_query_create("AT+CGSN\r\n", "\r\n([0-9]+)\r\n\r\nOK\r\n");
    mc7700_query_execute(thread_priv.q, q);

    /* cutting IMEI number from the reply */
    if(q->answer)
        res = rpc_create(
            TYPE_RESPONSE, __func__,
            (uint8_t*)q->answer + q->re_subs[1].rm_so,
            q->re_subs[1].rm_eo - q->re_subs[1].rm_so
        );

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_imsi(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;

    q = mc7700_query_create("AT+CIMI\r\n", "\r\n([0-9]+)\r\n\r\nOK\r\n");
    mc7700_query_execute(thread_priv.q, q);

    /* cutting IMEI number from the reply */
    if(q->answer)
        res = rpc_create(
            TYPE_RESPONSE, __func__,
            (uint8_t*)q->answer + q->re_subs[1].rm_so,
            q->re_subs[1].rm_eo - q->re_subs[1].rm_so
        );

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_signal_quality(modem_client_thread_t* priv, rpc_packet_t* p)
{
    modem_signal_quality_t sq;
    rpc_packet_t *res = NULL;
    char rssi[16], ber[16];
    mc7700_query_t *q;
    int nrssi, nber;

    q = mc7700_query_create("AT+CSQ\r\n", "\r\n\\+CSQ: ([0-9]+),([0-9]+)\r\n\r\nOK\r\n");
    mc7700_query_execute(thread_priv.q, q);

    /* cutting IMEI number from the reply */
    if(q->answer)
    {
        memset(rssi, 0, sizeof(rssi));
        memset(ber, 0, sizeof(ber));

        memcpy(rssi, q->answer + q->re_subs[1].rm_so, q->re_subs[1].rm_eo - q->re_subs[1].rm_so);
        memcpy(ber, q->answer + q->re_subs[2].rm_so, q->re_subs[2].rm_eo - q->re_subs[2].rm_so);

        nrssi = atoi(rssi);
        nber = atoi(ber);

        /* calculation dBm */
        sq.dbm = (nrssi > 31) ? 0 : nrssi * 2 - 113;

        /* calculation signal level */
        sq.level = !!sq.dbm;
        sq.level += (sq.dbm >= -95);
        sq.level += (sq.dbm >= -85);
        sq.level += (sq.dbm >= -73);
        sq.level += (sq.dbm >= -65);

        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&sq, sizeof(sq));
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_network_time(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;
    char dt[24];
    struct tm tm;
    time_t t;

     q = mc7700_query_create("AT!TIME?\r\n", "!TIME:.*\r\n([0-9,/]+\r\n[0-9,:]+) \\(local\\)\r\n[0-9,/]+\r\n[0-9,:]+ \\(UTC\\)\r\n\r\n\r\nOK\r\n");
    mc7700_query_execute(thread_priv.q, q);

    /* cutting TIME from the reply */
    if(q->answer)
    {
        memcpy(dt, q->answer + q->re_subs[1].rm_so, q->re_subs[1].rm_eo - q->re_subs[1].rm_so);
        dt[q->re_subs[1].rm_eo - q->re_subs[1].rm_so] = 0;

        /* parsing date and time */
        strptime(dt, "%Y/%m/%d\r\n%H:%M:%S", &tm);

        t = mktime(&tm);

        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&t, sizeof(t));
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_operator_name(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;
    int res_ok;

    /* setup format of +COPS as a string */
    q = mc7700_query_create("AT+COPS=3,0\r\n", "\r\nOK\r\n");
    mc7700_query_execute(thread_priv.q, q);
    res_ok = !!q->answer;
    mc7700_query_destroy(q);

    if(!res_ok)
        return(res);

    q = mc7700_query_create("AT+COPS?\r\n", "\r\n\\+COPS: [0-9],[0-9],\"(.+)\",[0-9]\r\n\r\nOK\r\n");
    mc7700_query_execute(thread_priv.q, q);

    /* cutting Operator name from the answer */
    if(q->answer)
        res = rpc_create(
            TYPE_RESPONSE, __func__,
            (uint8_t*)q->answer + q->re_subs[1].rm_so,
            q->re_subs[1].rm_eo - q->re_subs[1].rm_so
        );

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_network_registration(modem_client_thread_t* priv, rpc_packet_t* p)
{
    modem_network_reg_t nr = MODEM_NETWORK_REG_UNKNOWN;
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;
    int nnr;

#ifdef __HW_C1KMBR
    q = mc7700_query_create("AT+CEREG?\r\n", "\r\n\\+CEREG: [0-9],([0-9])\r\n\r\nOK\r\n");
#else
    q = mc7700_query_create("AT+CREG?\r\n", "\r\n\\+CREG: [0-9],([0-9])\r\n\r\nOK\r\n");
#endif /* __HW_C1KMBR */

    mc7700_query_execute(thread_priv.q, q);

    if(q->answer)
    {
        /* cutting registration status from the reply and check value */
        /* fast ASCII digit conversion (char - 0x30) */
        nnr = *(q->answer + q->re_subs[1].rm_so) - 0x30;
        nr = (nnr >= 0 && nnr <= 5) ? nnr : MODEM_NETWORK_REG_UNKNOWN;
    }

    res = rpc_create(TYPE_RESPONSE, __func__, &nr, sizeof(nr));

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_network_type(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;

    q = mc7700_query_create("AT*CNTI=0\r\n", "\r\n\\*CNTI: 0,(.+)\r\n\r\nOK\r\n");

    mc7700_query_execute(thread_priv.q, q);

    /* cutting Operator name from the answer */
    if(q->answer)
        res = rpc_create(
            TYPE_RESPONSE, __func__,
            (uint8_t*)q->answer + q->re_subs[1].rm_so,
            q->re_subs[1].rm_eo - q->re_subs[1].rm_so
        );

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_change_pin(modem_client_thread_t* priv, rpc_packet_t* p)
{
    modem_change_pin_t *pc = (modem_change_pin_t*)p->data;
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;
    char cmd[0x100];

    if(p->hdr.data_len != sizeof(*pc))
        return(res);

    snprintf(cmd, sizeof(cmd), "AT+CPWD=\"SC\",\"%s\",\"%s\"\r\n", pc->old_pin, pc->new_pin);

    q = mc7700_query_create(cmd, "\r\nOK\r\n");

    mc7700_query_execute(thread_priv.q, q);

    /* cutting Operator name from the answer */
    if(q->answer)
        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)q->answer, strlen(q->answer));

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_fw_version(modem_client_thread_t* priv, rpc_packet_t* p)
{
    modem_fw_version_t fw_info;
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;
    char firmware[0x100];
    char release[0x100];
    struct tm tm;

    q = mc7700_query_create("AT+CGMR\r\n", "\r\n.*(SWI.*) .* .* ([0-9,/]+ [0-9,:]+)\r\n\r\nOK\r\n");

    mc7700_query_execute(thread_priv.q, q);

    /* cutting Operator name from the answer */
    if(q->answer)
    {
        memcpy(firmware, q->answer + q->re_subs[1].rm_so, q->re_subs[1].rm_eo - q->re_subs[1].rm_so);
        firmware[q->re_subs[1].rm_eo - q->re_subs[1].rm_so] = 0;

        memcpy(release, q->answer + q->re_subs[2].rm_so, q->re_subs[2].rm_eo - q->re_subs[2].rm_so);
        release[q->re_subs[2].rm_eo - q->re_subs[2].rm_so] = 0;

        /* create result */
        strncpy(fw_info.firmware, firmware, sizeof(fw_info.firmware) - 1);
        fw_info.firmware[sizeof(fw_info.firmware) - 1] = 0;

        /* parsing date and time */
        strptime(release, "%Y/%m/%d\r\n%H:%M:%S", &tm);
        fw_info.release = mktime(&tm);

        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&fw_info, sizeof(fw_info));
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_info(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    modem_info_t* mi;

    mi = usb_device_get_info(priv->port);
    res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)mi, sizeof(*mi) ? sizeof(*mi) : 0);

    free(mi);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_operator_scan(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    modem_oper_t *opers;
    mc7700_query_t *q;
    int nopers = 0;
    char *sopers;

    q = mc7700_query_create("AT+COPS=?\r\n", "\r\n\\+COPS: (.+),,\\(.+\\),\\(.+\\)\r\n\r\nOK\r\n");
    q->timeout = 120;

    mc7700_query_execute(thread_priv.q, q);

    /* cutting operators from the answer */
    if(q->answer)
    {
        if((sopers = malloc(q->re_subs[1].rm_eo - q->re_subs[1].rm_so + 1)))
        {
            /* parsing operator list */
            __REGMATCH_CUT(sopers, q->answer, q->re_subs[1]);
            nopers = at_parse_cops_list(sopers, &opers);

            res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)opers, sizeof(modem_oper_t) * nopers);

            free(opers);
            free(sopers);
        }
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_at_command(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;
    char *query;

    /* formating query */
    if(!(query = malloc(p->hdr.data_len + 1 + 2))) /* +2 for "\r\n" */
        return(res);

    memcpy(query, p->data, p->hdr.data_len);
    strcpy(query + p->hdr.data_len, "\r\n");

    q = mc7700_query_create(query, "OK\r\n");
    q->timeout = 5;

    free(query);

    mc7700_query_execute(thread_priv.q, q);

    /* cutting Operator name from the answer */
    if(q->answer)
        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)q->answer, strlen(q->answer));

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_cell_id(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    mc7700_query_t *q;

    q = mc7700_query_create("AT!GSMINFO?\r\n", "\r\n!GSMINFO:.*Cell ID: +([0-9]+).*\r\n\r\nOK\r\n");
    mc7700_query_execute(thread_priv.q, q);

    /* cutting Cell ID number from the reply */
    if(q->answer)
        res = rpc_create(
            TYPE_RESPONSE, __func__,
            (uint8_t*)q->answer + q->re_subs[1].rm_so,
            q->re_subs[1].rm_eo - q->re_subs[1].rm_so
        );

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

const rpc_function_info_t rpc_functions[] = {
    {"modem_find_first", modem_find_first_packet},
    {"modem_find_next", modem_find_next_packet},
    {"modem_open_by_port", modem_open_by_port},
    {"modem_close", modem_close},
    {"modem_get_imei", modem_get_imei},
    {"modem_get_signal_quality", modem_get_signal_quality},
    {"modem_get_network_time", modem_get_network_time},
    {"modem_get_imsi", modem_get_imsi},
    {"modem_get_operator_name", modem_get_operator_name},
    {"modem_network_registration", modem_network_registration},
    {"modem_get_network_type", modem_get_network_type},
    {"modem_change_pin", modem_change_pin},
    {"modem_get_fw_version", modem_get_fw_version},
    {"modem_get_info", modem_get_info},
    {"modem_operator_scan", modem_operator_scan},
    {"modem_at_command", modem_at_command},
    {"modem_get_cell_id", modem_get_cell_id},
    {{0, 0}},
};

/*------------------------------------------------------------------------*/

void* ThreadWrapper(void* prm)
{
    const rpc_function_info_t *rpc_func;
    modem_client_thread_t* priv = prm;
    rpc_packet_t *p_in = NULL, *p_out;

    while(!priv->terminate && (p_in = rpc_recv(priv->sock)))
    {
        rpc_print(p_in);

        if(p_in->hdr.type != TYPE_QUERY)
        {
            rpc_free(p_in);
            continue;
        }

        rpc_func = rpc_functions;
        p_out = NULL;

        while(rpc_func->func)
        {
            if(strcmp(rpc_func->name, p_in->func) == 0)
            {
                /* execute function */
                p_out = rpc_func->func(priv, p_in);

                break;
            }

            ++ rpc_func;
        }

        if(!p_out)
            /* function failed, create NULL result */
            p_out = rpc_create(TYPE_RESPONSE, p_in->func, NULL, 0);

        rpc_send(priv->sock, p_out);
        rpc_print(p_out);
        rpc_free(p_out);
        rpc_free(p_in);
    }

    /* cleanup resources */
    close(priv->sock);

    if(priv->dir)
        closedir(priv->dir);

    free(priv);

    return(NULL);
}

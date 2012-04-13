#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "lib/log.h"
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

    int reg_need;
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
    rpc_packet_t *res = NULL;
    int path_len, modem = 0;
    int modem_wakeup_tries;
    char port[0x100] = {0};
    char tty[0x100] = {0};

    path_len = p->hdr.data_len >= sizeof(port) ? sizeof(port) - 1 : p->hdr.data_len;
    memcpy(port, p->data, path_len);
    port[path_len] = 0;

    modem_wakeup_tries = 3;

    while(!modem_get_at_port_name(port, tty, sizeof(tty)) && modem_wakeup_tries)
    {
#ifdef __MODEMD_DEBUG
        log_dbg("Wait modem wakeup on port %s\n", port);
#endif
        port_power(port, 1);
        sleep(20);
        -- modem_wakeup_tries;
    }

    if(*tty)
    {
#ifdef __MODEMD_DEBUG
        log_dbg("==== %s -> %s\n", port, tty);
#endif
        strncpy(priv->port, port, sizeof(priv->port) - 1);
        priv->port[sizeof(priv->port) - 1] = 0;

        modem = mc7700_open(port, tty);

        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&modem, sizeof(modem));
    }

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

    if(*mc7700_thread_priv.imei)
        res = rpc_create(TYPE_RESPONSE, __func__,
            (uint8_t*)mc7700_thread_priv.imei,
            strlen(mc7700_thread_priv.imei));

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_imsi(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;

    if(*mc7700_thread_priv.imsi)
        res = rpc_create(TYPE_RESPONSE, __func__,
            (uint8_t*)mc7700_thread_priv.imsi,
            strlen(mc7700_thread_priv.imsi));

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_signal_quality(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;

    /* if signal present */
    if(mc7700_thread_priv.sq.dbm)
        res = rpc_create(TYPE_RESPONSE, __func__,
            (uint8_t*)&mc7700_thread_priv.sq,
            sizeof(mc7700_thread_priv.sq));

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
    mc7700_query_execute(mc7700_thread_priv.q, q);

    /* cutting TIME from the reply */
    if(q->answer)
    {
        __REGMATCH_N_CUT(dt, sizeof(dt), q->answer, q->re_subs[1])

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

    if(*mc7700_thread_priv.oper)
        res = rpc_create(TYPE_RESPONSE, __func__,
            (uint8_t*)mc7700_thread_priv.oper,
            strlen(mc7700_thread_priv.oper)
        );

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_network_registration(modem_client_thread_t* priv, rpc_packet_t* p)
{
    return(rpc_create(TYPE_RESPONSE, __func__, &mc7700_thread_priv.reg, sizeof(mc7700_thread_priv.reg)));
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_network_type(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;

    if(*mc7700_thread_priv.network_type)
        res = rpc_create(
            TYPE_RESPONSE, __func__,
            (uint8_t*)mc7700_thread_priv.network_type,
            strlen(mc7700_thread_priv.network_type)
        );

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

    mc7700_query_execute(mc7700_thread_priv.q, q);

    if(q->answer)
        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)q->answer, strlen(q->answer));

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_fw_version(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;

    if(mc7700_thread_priv.fw_info.release)
        res = rpc_create(TYPE_RESPONSE, __func__,
            (uint8_t*)&mc7700_thread_priv.fw_info,
            sizeof(mc7700_thread_priv.fw_info));

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_info(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    modem_info_t* mi;

    if((mi = usb_device_get_info(priv->port)))
        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)mi, sizeof(*mi));

    free(mi);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_operator_scan(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    modem_oper_t *opers;
    int nopers = 0;

    if((nopers = at_operator_scan(mc7700_thread_priv.q, &opers)) > 0)
        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)opers, sizeof(modem_oper_t) * nopers);

    free(opers);

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

    mc7700_query_execute(mc7700_thread_priv.q, q);

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
    char cell_ids[0x100];
    mc7700_query_t *q;
    int32_t cell_id;

    q = mc7700_query_create("AT!GSMINFO?\r\n", "!GSMINFO:.*\r\nCell ID:[\t]*([0-9]+)\r\n.*\r\nOK\r\n");
    mc7700_query_execute(mc7700_thread_priv.q, q);

    /* cutting Cell ID number from the reply */
    if(q->answer)
    {
        __REGMATCH_N_CUT(cell_ids, sizeof(cell_ids), q->answer, q->re_subs[1])

        cell_id = atoi(cell_ids);

        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&cell_id, sizeof(cell_id));
    }

    mc7700_query_destroy(q);

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_operator_scan_start(modem_client_thread_t* priv, rpc_packet_t* p)
{
    at_operator_scan_t *at_priv;
    rpc_packet_t* res = NULL;
    int thread_res, file_len;

    if(mc7700_thread_priv.thread_scan)
        goto exit;

    if(!(at_priv = malloc(sizeof(*at_priv))))
        goto exit;

    /* cutting file name */
    file_len = p->hdr.data_len >= sizeof(at_priv->file) ? sizeof(at_priv->file) - 1 : p->hdr.data_len;
    memcpy(at_priv->file, p->data, file_len);
    at_priv->file[file_len] = 0;

    at_priv->priv = &mc7700_thread_priv;

    /* creating thread with a query AT+COPS=? */
    thread_res = pthread_create(&mc7700_thread_priv.thread_scan, NULL, at_thread_operator_scan, at_priv);

    if(!thread_res)
        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&thread_res, sizeof(thread_res));
    else
        free(at_priv);

exit:
    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_operator_scan_is_running(modem_client_thread_t* priv, rpc_packet_t* p)
{
    rpc_packet_t *res = NULL;
    int8_t scan_res = -1;
    int kill_res = -1;
    void *thread_res;

    if(mc7700_thread_priv.thread_scan)
        kill_res = pthread_kill(mc7700_thread_priv.thread_scan, 0);

    if(kill_res == ESRCH)
    {
        pthread_join(mc7700_thread_priv.thread_scan, &thread_res);
        mc7700_thread_priv.thread_scan = 0;
        scan_res = 0;
    }
    else if(kill_res == 0)
        scan_res = 1;

    if(scan_res >= 0)
        res = rpc_create(TYPE_RESPONSE, __func__, (uint8_t*)&scan_res, sizeof(scan_res));

    return(res);
}

/*------------------------------------------------------------------------*/

rpc_packet_t* modem_get_last_error(modem_client_thread_t* priv, rpc_packet_t* p)
{
    if(mc7700_thread_priv.locked && mc7700_thread_priv.last_error == -1)
        mc7700_thread_priv.last_error = 258; /* we are busy now */

    return
    (
        rpc_create(TYPE_RESPONSE, __func__,
            (uint8_t*)&mc7700_thread_priv.last_error,
            sizeof(mc7700_thread_priv.last_error))
    );
}

/*------------------------------------------------------------------------*/

const rpc_function_info_t rpc_functions[] = {
    {"modem_find_first", modem_find_first_packet},
    {"modem_find_next", modem_find_next_packet},
    {"modem_open_by_port", modem_open_by_port},
    {"modem_close", modem_close},
    {"modem_get_info", modem_get_info},
    {"modem_get_last_error", modem_get_last_error},
    {"modem_get_imei", modem_get_imei,                                 0},
    {"modem_change_pin", modem_change_pin,                             0},
    {"modem_get_fw_version", modem_get_fw_version,                     0},
    {"modem_network_registration", modem_network_registration,         0},
    {"modem_get_imsi", modem_get_imsi,                                 0},
    {"modem_operator_scan_start", modem_operator_scan_start,           0},
    {"modem_operator_scan_is_running", modem_operator_scan_is_running, 0},
    {"modem_get_signal_quality", modem_get_signal_quality,             1},
    {"modem_operator_scan", modem_operator_scan,                       1},
    {"modem_at_command", modem_at_command,                             1},
    {"modem_get_network_time", modem_get_network_time,                 1},
    {"modem_get_operator_name", modem_get_operator_name,               1},
    {"modem_get_network_type", modem_get_network_type,                 1},
    {"modem_get_cell_id", modem_get_cell_id,                           1},
    {{0, 0, 0}},
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
                if(rpc_func->reg_need && mc7700_thread_priv.locked)
                {
                    log_warn("Denied function %s Registration is not complete\n",
                        rpc_func->name);

                    break;
                }

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

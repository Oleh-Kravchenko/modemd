#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <regex.h>

#include "lib/log.h"
#include "queue.h"
#include "mc7700.h"
#include "lib/utils.h"
#include "lib/modem_int.h"

/*------------------------------------------------------------------------*/

#define THREAD_WAIT 1

/*------------------------------------------------------------------------*/

thread_queue_t mc7700_thread_priv;

/*------------------------------------------------------------------------*/

void* mc7700_thread_write(void* prm)
{
    thread_queue_t *priv = prm;
    size_t buf_len;
    void* buf;

    while(!priv->terminate)
    {
        /* receive pointer to query */
        if(queue_wait_pop(priv->q, THREAD_WAIT, &buf, &buf_len))
            continue;

        if(buf_len != sizeof(void**))
        {
            free(buf);
            continue;
        }

        /* resolve pointer to query */
        priv->query = (mc7700_query_t*)*((mc7700_query_t**)buf);
        free(buf);

#ifdef __MODEMD_DEBUG
        log_info("write(%d): %s", strlen(priv->query->query), priv->query->query);
#endif

        write(priv->fd, priv->query->query, strlen(priv->query->query));

        /* wait for answer */
        pthread_mutex_lock(&priv->mutex);
        pthread_cond_wait(&priv->processed, &priv->mutex);
        pthread_mutex_unlock(&priv->mutex);
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

void* mc7700_thread_read(void* prm)
{
    thread_queue_t *priv = prm;
    struct pollfd p = {priv->fd, POLLIN, 0};
    int buf_len = 0, res = 0, giveup = 0, re_res;
    char buf[0xffff];
    regex_t re;

    while(!priv->terminate)
    {
        res = poll(&p, 1, (THREAD_WAIT) * 1000);

        if(res > 0 && p.revents & POLLIN)
        {
            res = read(priv->fd, buf + buf_len, sizeof(buf) - buf_len - 1);

            if(res == -1)
            {
                log_err("failed read() %d\n", res);

                continue;
            }

            buf_len += res;

            if(buf_len > 0 && priv->query)
            {
                buf[buf_len] = 0;

#ifdef __MODEMD_DEBUG
                log_info("read(%d): %s", buf_len, buf);
#endif /*__MODEMD_DEBUG */

                regcomp(&re, priv->query->answer_reg, REG_EXTENDED);
                priv->query->n_subs = re.re_nsub + 1;
                priv->query->re_subs = malloc(sizeof(regmatch_t) * priv->query->n_subs);
                re_res = regexec(&re, buf, priv->query->n_subs, priv->query->re_subs, 0);

                if(re_res)
                {
#if 0
                    char re_err[0x100];

                    regerror(re_res, &re, re_err, sizeof(re_err));

                    log_err("regexec() %s [\n%s\n]\n", re_err, buf);
#endif
                    free(priv->query->re_subs);
                    priv->query->re_subs = NULL;
                    regfree(&re);

                    priv->query->error = at_parse_error(buf);

                    if(priv->query->error == -1)
                        /* no error detected, proceed collecting data */
                        continue;
                }

                regfree(&re);

                if(priv->query->error == -1)
                {
                    priv->query->answer = malloc(buf_len + 1);
                    strncpy(priv->query->answer, buf, buf_len );
                    priv->query->answer[buf_len] = 0;
                }
            }
        }

        if
        (priv->query &&
            (
                giveup >= priv->query->timeout ||
                priv->query->answer ||
                priv->query->error != -1
            )
        )
        {
            if(giveup >= priv->query->timeout)
            {
                log_info("Command %s expired after %d second(s)", priv->query->query, giveup);
                log_info("No match %s", buf);
            }
            else if(priv->query->error > 0) /* ignore general error */
            {
                log_info("CME ERROR: %d", priv->query->error);
                priv->last_error = priv->query->error;
            }

            /* reporting about reply */
            pthread_mutex_lock(&priv->query->cond_m);
            pthread_cond_signal(&priv->query->cond);
            pthread_mutex_unlock(&priv->query->cond_m);

            buf_len = 0;
            giveup = 0;
            priv->query = NULL;

            /* notify writing thread */
            pthread_mutex_lock(&priv->mutex);
            pthread_cond_signal(&priv->processed);
            pthread_mutex_unlock(&priv->mutex);
        }
        else if(priv->query)
            ++ giveup;
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

void* mc7700_thread_reg(void* prm)
{
    thread_queue_t *priv = prm;
    char s[0x100];
    int res_ok;

    at_get_fw_version(priv->q, &priv->fw_info);
    at_get_imei(priv->q, priv->imei, sizeof(priv->imei));

    /* waiting for config */
    while(!priv->conf.from_file)
    {
        mc7700_read_config(priv->port, &priv->conf);
        log_dbg("Waiting configuration for %s port\n", priv->port);

        sleep(5);
    }

    at_raw_ok(priv->q, "ATE0\r\n");
    at_raw_ok(priv->q, "AT+CMEE=1\r\n");
    at_raw_ok(priv->q, "AT+CFUN=1\r\n");
#ifdef __HW_C1KMBR
    snprintf(s, sizeof(s), "AT!BAND=%02X\r\n", priv->conf.frequency_band);
    res_ok = at_raw_ok(priv->q, s);
#endif /* __HW_C1KMBR */

    if(*priv->conf.data.apn)
    {
        snprintf(s, sizeof(s), "AT+CGDCONT=3,\"IPV4V6\",\"%s\"\r\n", priv->conf.data.apn);
        res_ok = at_raw_ok(priv->q, s);

        if(priv->conf.data.auth != PPP_NONE)
        {
            snprintf(s, sizeof(s), "AT$QCPDPP=3,%d,\"%s\",\"%s\"\r\n",
                priv->conf.data.auth, priv->conf.data.password, priv->conf.data.username);

            res_ok = at_raw_ok(priv->q, s);
        }
        else
        {
            res_ok = at_raw_ok(priv->q, "AT$QCPDPP=3,0\r\n");
        }
    }

    /* pin && puk handling */
    res_ok = -1;

    switch(at_cpin_state(priv->q))
    {
        case MODEM_CPIN_STATE_PIN:
            if(*priv->conf.pin)
                res_ok = at_cpin_pin(priv->q, priv->conf.pin);
            else
                priv->last_error = 11;
            break;

        case MODEM_CPIN_STATE_PUK:
            if(*priv->conf.pin && *priv->conf.puk)
                res_ok = at_cpin_puk(priv->q, priv->conf.puk, priv->conf.pin);
            else
                priv->last_error = 12;
            break;

        case MODEM_CPIN_STATE_READY:
            res_ok = 0;
            break;

        default:
            res_ok = -1;
            break;
    }

    if(res_ok == -1)
    {
        /* pin && puk failed */
        priv->locked = 1;

        return(NULL);
    }

    /* waiting for IMSI ready */
    while(!*priv->imsi && !priv->terminate_reg)
    {
        log_dbg("Waiting for IMSI ready..\n");

        sleep(1);

        at_get_imsi(priv->q, priv->imsi, sizeof(priv->imsi));
    }

    if(priv->conf.operator_number)
    {
        snprintf(s, sizeof(s), "AT+COPS=1,2,%d,%d\r\n", priv->conf.operator_number, priv->conf.access_technology);
        at_raw_ok(priv->q, s);
    }
    else
        at_raw_ok(priv->q, "AT+COPS=0\r\n");

    while(!priv->terminate_reg)
    {
        sleep(10);

        priv->reg = at_network_registration(priv->q);

        /* if roaming disabled */
        if(MODEM_NETWORK_REG_ROAMING == priv->reg && !priv->conf.roaming)
            /* set registration status as a denied */
            priv->reg = MODEM_NETWORK_REG_DENIED;

        switch(priv->reg)
        {
            case MODEM_NETWORK_REG_HOME:
            case MODEM_NETWORK_REG_ROAMING:
                priv->locked = 0;
                break;

            case MODEM_NETWORK_REG_FAILED:
            case MODEM_NETWORK_REG_SEARCHING:
            case MODEM_NETWORK_REG_DENIED:
            case MODEM_NETWORK_REG_UNKNOWN:
            default:
                memset(&priv->sq, 0, sizeof(priv->sq));
                memset(&priv->network_type, 0, sizeof(priv->network_type));
                memset(&priv->oper, 0, sizeof(priv->oper));

                priv->locked = 1;

                continue;
        }

        at_get_signal_quality(priv->q, &priv->sq);

        at_get_network_type(priv->q, priv->network_type, sizeof(priv->network_type));

        at_get_operator_name(priv->q, priv->oper, sizeof(priv->oper));
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

void mc7700_read_config(const char* port, modem_conf_t* conf)
{
    char s[0x100];
    FILE *f;

    /* default values */
    *conf->pin = 0;
    *conf->puk = 0;
    *conf->data.apn = 0;
    *conf->data.apn = 0;
    conf->data.auth = PPP_NONE;
    *conf->data.username = 0;
    *conf->data.password = 0;
    conf->roaming = 0;
    conf->operator_number = 0;
    conf->access_technology = 0;
    conf->frequency_band = 0;
    conf->from_file = 0;

    /* path to config */
    snprintf(s, sizeof(s), "/etc/modemd/%s/conf", port);

    if(!(f = fopen(s, "r")))
    {
        perror(s);
        return;
    }

    conf->from_file = 1;

    while(!feof(f))
    {
        if(!fgets(s, sizeof(s), f))
            continue;

#define CONF_PIN     "pin="
#define CONF_PUK     "puk="
#define CONF_APN     "apn="
#define CONF_AUTH    "auth="
#define CONF_USER    "username="
#define CONF_PASS    "password="
#define CONF_ROAMING "roaming_enable=yes"
#define CONF_OPER    "operator_number="
#define CONF_ACT     "access_technology="
#define CONF_BAND    "frequency_band="

        if(strstr(s, CONF_PIN) == s)
        {
            strncpy(conf->pin, s + strlen(CONF_PIN), sizeof(conf->pin) - 1);
            conf->pin[sizeof(conf->pin) - 1] = 0;
            mystrtrmr_a(conf->pin);
        }
        else if(strstr(s, CONF_PUK) == s)
        {
            strncpy(conf->puk, s + strlen(CONF_PUK), sizeof(conf->puk) - 1);
            conf->puk[sizeof(conf->puk) - 1] = 0;
            log_dbg("PUK: [%s]\n", conf->puk);
            mystrtrmr_a(conf->puk);
        }
        else if(strstr(s, CONF_APN) == s)
        {
            strncpy(conf->data.apn, s + strlen(CONF_APN), sizeof(conf->data.apn) - 1);
            conf->data.apn[sizeof(conf->data.apn) - 1] = 0;
            mystrtrmr_a(conf->data.apn);
        }
        else if(strstr(s, CONF_AUTH) == s)
        {
            conf->data.auth = atoi(s + strlen(CONF_AUTH));
        }
        else if(strstr(s, CONF_USER) == s)
        {
            strncpy(conf->data.username, s + strlen(CONF_USER), sizeof(conf->data.username) - 1);
            conf->data.username[sizeof(conf->data.username) - 1] = 0;
            mystrtrmr_a(conf->data.username);
        }
        else if(strstr(s, CONF_PASS) == s)
        {
            strncpy(conf->data.password, s + strlen(CONF_PASS), sizeof(conf->data.password) - 1);
            conf->data.password[sizeof(conf->data.password) - 1] = 0;
            mystrtrmr_a(conf->data.password);
        }
        else if(strstr(s, CONF_ROAMING))
        {
            conf->roaming = 1;
        }
        else if(strstr(s, CONF_OPER) == s)
        {
            conf->operator_number = atoi(s + strlen(CONF_OPER));
        }
        else if(strstr(s, CONF_ACT) == s)
        {
            conf->access_technology = atoi(s + strlen(CONF_ACT));
        }
        else if(strstr(s, CONF_BAND) == s)
        {
            conf->frequency_band = atoi(s + strlen(CONF_BAND));
        }
    }

    fclose(f);

    log_dbg("PIN: %s PUK: %s\n"
        "APN: %s\n"
        "Auth: %d\n"
        "Username: %s\n"
        "Password: %s\n"
        "roaming: %d\n"
        "operator_number: %d\n"
        "access_technology: %d\n"
        "frequency_band: %d\n"
        "from_file: %d\n",

        conf->pin,
        conf->puk,
        conf->data.apn,
        conf->data.auth,
        conf->data.username,
        conf->data.password,
        conf->roaming,
        conf->operator_number,
        conf->access_technology,
        conf->frequency_band,
        conf->from_file
    );
}

/*------------------------------------------------------------------------*/

int mc7700_open(const char *port, const char *tty)
{
    if(mc7700_thread_priv.mc7700_clients ++)
        return(mc7700_thread_priv.mc7700_clients);

    strncpy(mc7700_thread_priv.port, port, sizeof(mc7700_thread_priv.port) - 1);
    mc7700_thread_priv.port[sizeof(mc7700_thread_priv.port) - 1] = 0;

    strncpy(mc7700_thread_priv.tty, tty, sizeof(mc7700_thread_priv.tty) - 1);
    mc7700_thread_priv.tty[sizeof(mc7700_thread_priv.tty) - 1] = 0;

    mc7700_thread_priv.fd = serial_open(tty, O_RDWR);
    mc7700_thread_priv.q = queue_create();
    mc7700_thread_priv.terminate = 0;
    mc7700_thread_priv.terminate_reg = 0;
    mc7700_thread_priv.locked = 1;
    mc7700_thread_priv.query = NULL;
    mc7700_thread_priv.last_error = -1;
    mc7700_thread_priv.reg = MODEM_NETWORK_REG_SEARCHING;
    pthread_cond_init(&mc7700_thread_priv.processed, NULL);
    pthread_mutex_init(&mc7700_thread_priv.mutex, NULL);

    mc7700_thread_priv.thread_scan = 0;

    mc7700_read_config(port, &mc7700_thread_priv.conf);

    /* creating registration thread */
    pthread_create(&mc7700_thread_priv.thread_reg, NULL, mc7700_thread_reg, &mc7700_thread_priv);

    /* creating reading thread */
    pthread_create(&mc7700_thread_priv.thread_read, NULL, mc7700_thread_read, &mc7700_thread_priv);

    /* creating write thread */
    pthread_create(&mc7700_thread_priv.thread_write, NULL, mc7700_thread_write, &mc7700_thread_priv);

    return(mc7700_thread_priv.mc7700_clients);
}

/*------------------------------------------------------------------------*/

void mc7700_destroy(void)
{
    void* thread_res;

    if(mc7700_thread_priv.mc7700_clients <= 0)
        return;

    if(!(-- mc7700_thread_priv.mc7700_clients))
    {
        if(mc7700_thread_priv.thread_scan)
            pthread_join(mc7700_thread_priv.thread_scan, &thread_res);

        mc7700_thread_priv.terminate_reg = 1;

        /* waiting for termination thread_reg */
        pthread_join(mc7700_thread_priv.thread_reg, &thread_res);

        mc7700_thread_priv.terminate = 1;

        pthread_join(mc7700_thread_priv.thread_write, &thread_res);
        pthread_join(mc7700_thread_priv.thread_read, &thread_res);

        /* cleanup used resources */
        close(mc7700_thread_priv.fd);
        queue_destroy(mc7700_thread_priv.q);
        pthread_mutex_destroy(&mc7700_thread_priv.mutex);
        pthread_cond_destroy(&mc7700_thread_priv.processed);
    }
}

/*------------------------------------------------------------------------*/

mc7700_query_t* mc7700_query_create(const char* q, const char* reply_re)
{
    mc7700_query_t* res;

    if(!(res = malloc(sizeof(*res))))
        goto err;

    if(!(res->query = malloc(strlen(q) + 1)))
        goto err_q;

    strncpy(res->query, q, strlen(q) + 1);
    res->query[strlen(q)] = 0;

    if(!(res->answer_reg = malloc(strlen(reply_re) + 1)))
        goto err_a;

    strncpy(res->answer_reg, reply_re, strlen(reply_re) + 1);
    res->answer_reg[strlen(reply_re)] = 0;

    res->answer = NULL;
    res->re_subs = NULL;
    res->n_subs = 0;
    res->timeout = 2; /* default timeout for command */
    res->error = -1;  /* -1 is no error */

    pthread_cond_init(&res->cond, NULL);
    pthread_mutex_init(&res->cond_m, NULL);

    goto exit;

err_a:
    free(res->query);

err_q:
    free(res);
    res = NULL;

err:
exit:
    return(res);
}

/*------------------------------------------------------------------------*/

int mc7700_query_execute(queue_t* queue, mc7700_query_t* query)
{
    int res;

    if((res = queue_add(queue, &query, sizeof(mc7700_query_t**))))
        goto err;

    /* wait for processing */
    pthread_mutex_lock(&query->cond_m);
    pthread_cond_wait(&query->cond, &query->cond_m);
    pthread_mutex_unlock(&query->cond_m);

err:
    return(res);
}

/*------------------------------------------------------------------------*/

void mc7700_query_destroy(mc7700_query_t* q)
{
    if(!q)
        return;

    free(q->answer);
    free(q->answer_reg);
    free(q->query);
    pthread_cond_destroy(&q->cond);
    pthread_mutex_destroy(&q->cond_m);
    free(q->re_subs);
    free(q);
}

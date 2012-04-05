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
        printf("(II) write [\n%s\n]\n", priv->query->query);
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
                printf("(EE) failed read() %d\n", res);

                continue;
            }

            buf_len += res;

            if(buf_len > 0 && priv->query)
            {
                buf[buf_len] = 0;

#ifdef __MODEMD_DEBUG
#if 0
                if(strncmp(query->query, buf, buf_len) == 0)
                    printf("(II) Allowed echo commands is detected!\n");
#endif
                printf("(II) read(%d) [\n%s\n]\n\n", buf_len, buf);
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

                    printf("(EE) regexec() %s [\n%s\n]\n", re_err, buf);
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

#ifdef __MODEMD_DEBUG
                printf("(II) Matched\n");
#endif

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
#ifdef __MODEMD_DEBUG
            if(giveup >= priv->query->timeout)
            {
                printf("(II) Command [%s] timeout after %d second(s)\n", priv->query->query, giveup);
                printf("(EE) No match [\n%s\n]\n", buf);
            }
            else if(priv->query->error != -1)
            {
                printf("(EE) CME ERROR: %d\n", priv->query->error);
            }
#endif
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

int mc7700_open(const char *port)
{
    if(mc7700_thread_priv.mc7700_clients ++)
        return(mc7700_thread_priv.mc7700_clients);

    mc7700_thread_priv.fd = serial_open(port, O_RDWR);
    mc7700_thread_priv.q = queue_create();
    mc7700_thread_priv.terminate = 0;
    mc7700_thread_priv.locked = 0;
    mc7700_thread_priv.query = NULL;
    pthread_cond_init(&mc7700_thread_priv.processed, NULL);
    pthread_mutex_init(&mc7700_thread_priv.mutex, NULL);

    mc7700_thread_priv.thread_scan = 0;

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

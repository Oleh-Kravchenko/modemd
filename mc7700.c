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

/*------------------------------------------------------------------------*/

#define THREAD_WAIT 1

/*------------------------------------------------------------------------*/

static int mc7700_clients = 0;

static pthread_t thread_write, thread_read;

thread_queue_t thread_priv;

static pthread_mutex_t mutex_mc7700 = PTHREAD_MUTEX_INITIALIZER;

mc7700_query_t* query = NULL;

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
        query = (mc7700_query_t*)*((mc7700_query_t**)buf);
        free(buf);

#ifdef __MODEMD_DEBUG
        printf("%s:%d %s() query [\n%s\n]\n", __FILE__, __LINE__, __func__, query->query);
#endif

        write(priv->fd, query->query, strlen(query->query));

        /* wait for answer */
        pthread_mutex_lock(&mutex_mc7700);
        pthread_cond_wait(&priv->processed, &mutex_mc7700);
        pthread_mutex_unlock(&mutex_mc7700);
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
            buf_len += read(priv->fd, buf + buf_len, sizeof(buf) - buf_len - 1);

            if(buf_len > 0 && query)
            {
                buf[buf_len] = 0;

#ifdef __MODEMD_DEBUG
#if 0
                if(strncmp(query->query, buf, buf_len) == 0)
                    printf("%s:%d %s() Allowed echo commands is detected!\n", __FILE__, __LINE__, __func__);
#endif
                printf("(II) readed(%d) [\n%s\n]\n\n", buf_len, buf);
#endif /*__MODEMD_DEBUG */

                regcomp(&re, query->answer_reg, REG_EXTENDED);
                query->n_subs = re.re_nsub + 1;
                query->re_subs = malloc(sizeof(regmatch_t) * query->n_subs);
                re_res = regexec(&re, buf, query->n_subs, query->re_subs, 0);

                if(re_res)
                {
#if 0
                    char re_err[0x100];

                    regerror(re_res, &re, re_err, sizeof(re_err));

                    printf("(EE) regexec() %s [\n%s\n]\n", re_err, buf);
#endif
                    free(query->re_subs);
                    query->re_subs = NULL;
                    regfree(&re);

                    continue;
                }

                regfree(&re);

#ifdef __MODEMD_DEBUG
                printf("(II) Matched\n");
#endif

                query->answer = malloc(buf_len + 1);
                strncpy(query->answer, buf, buf_len );
                query->answer[buf_len] = 0;
            }
        }

        if(query && (giveup >= query->timeout || query->answer))
        {
#ifdef __MODEMD_DEBUG
    if(giveup >= query->timeout)
    {
        printf("%s:%d %s() Command [%s] timeout after %d second(s)\n", __FILE__, __LINE__, __func__, query->query, giveup);

        printf("(EE) No match [\n%s\n]\n", buf);
    }
#endif

            /* reporting about reply */
            pthread_mutex_lock(&query->cond_m);
            pthread_cond_signal(&query->cond);
            pthread_mutex_unlock(&query->cond_m);

            buf_len = 0;
            giveup = 0;
            query = NULL;

            /* notify writing thread */
            pthread_mutex_lock(&mutex_mc7700);
            pthread_cond_signal(&priv->processed);
            pthread_mutex_unlock(&mutex_mc7700);
        }
        else if(query)
            ++ giveup;
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

int mc7700_open(const char *port)
{
    if(mc7700_clients ++)
        return(mc7700_clients);

    thread_priv.fd = serial_open(port, O_RDWR);
    thread_priv.q = queue_create();
    thread_priv.terminate = 0;
    pthread_cond_init(&thread_priv.processed, NULL);

    /* creating reading thread */
    pthread_create(&thread_read, NULL, mc7700_thread_read, &thread_priv);

    /* creating write thread */
    pthread_create(&thread_write, NULL, mc7700_thread_write, &thread_priv);

    return(mc7700_clients);
}

/*------------------------------------------------------------------------*/

void mc7700_destroy(void)
{
    void* thread_res;

    if(mc7700_clients <= 0)
        return;

    if(!(-- mc7700_clients))
    {
        thread_priv.terminate = 1;

        pthread_join(thread_write, &thread_res);
        pthread_join(thread_read, &thread_res);

        /* cleanup used resources */
        close(thread_priv.fd);
        queue_destroy(thread_priv.q);
        pthread_cond_destroy(&thread_priv.processed);
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

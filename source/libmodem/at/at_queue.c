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
#include <syslog.h>

#include "queue.h"
#include "at_queue.h"
#include "utils/re.h"
#include "utils/str.h"
#include "utils/file.h"
#include "at/at_utils.h"
#include "at/at_common.h"

/*------------------------------------------------------------------------*/

#define THREAD_WAIT 1

/*------------------------------------------------------------------------*/

void* at_queue_thread_write(void* prm)
{
    at_queue_t *priv = prm;
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
        priv->query = (at_query_t*)*((at_query_t**)buf);
        free(buf);

//        printf("(EE) write(%zd): [%s]\n", strlen(priv->query->cmd), priv->query->cmd);

        syslog(LOG_INFO | LOG_LOCAL7, "%s", priv->query->cmd);

        if(write(priv->fd, priv->query->cmd, strlen(priv->query->cmd)) == -1)
        {
            priv->query->error = 100; /* unknown error */
            priv->last_error = 100;

            /* reporting about failed command */
            pthread_mutex_lock(&priv->query->cond_m);
            pthread_cond_signal(&priv->query->cond);
            pthread_mutex_unlock(&priv->query->cond_m);

            priv->query = NULL;

            continue;
        }

        /* wait for answer */
        pthread_mutex_lock(&priv->mutex);
        pthread_cond_wait(&priv->processed, &priv->mutex);
        pthread_mutex_unlock(&priv->mutex);
    }

    return(NULL);
}

/*------------------------------------------------------------------------*/

void* at_queue_thread_read(void* prm)
{
    at_queue_t *priv = prm;
    struct pollfd p = {priv->fd, POLLIN, 0};
    int buf_len = 0, res = 0, giveup = 0, re_res = -1;
    char buf[0xffff];

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

//                printf("(EE) read(%d): %s", buf_len, buf);
                syslog(LOG_INFO | LOG_LOCAL7, "%s", buf);

				if((re_res = re_parse(buf, priv->query->re_res, &priv->query->nmatch, &priv->query->pmatch)))
                {
                    priv->query->error = at_parse_error(buf);

                    if(priv->query->error == -1)
                        /* no error detected, proceed collecting data */
                        continue;
                }
            }
        }

        if
        (priv->query &&
            (
                giveup >= priv->query->timeout ||
                re_res == 0 ||
                priv->query->error != -1
            )
        )
        {
            if(giveup >= priv->query->timeout)
            {
//                printf("(EE) Command %s expired after %d second(s)\n", priv->query->cmd, giveup);
//                printf("(EE) No match %s\n", buf);
                priv->query->error = 0;
				priv->last_error = 31;
            }
            else //if(priv->query->error > 0) /* ignore general error */
            {
//                printf("(EE) CME ERROR: %d %p\n", priv->query->error, (void*)priv);
                priv->last_error = priv->query->error;
            }

//			printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

            priv->query->result = malloc(buf_len + 1);
            strncpy(priv->query->result, buf, buf_len );
            priv->query->result[buf_len] = 0;

			re_res = -1;
			
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

at_queue_t* at_queue_open(const char *tty)
{
	at_queue_t *res;

    if((res = malloc(sizeof(*res))) == NULL)
		return(res);

    res->fd = serial_open(tty, O_RDWR);
    res->q = queue_create();
    res->terminate = 0;
    res->query = NULL;
    res->last_error = -1;

    pthread_cond_init(&res->processed, NULL);
    pthread_mutex_init(&res->mutex, NULL);

    /* creating reading thread */
    pthread_create(&res->thread_read, NULL, at_queue_thread_read, res);

    /* creating write thread */
    pthread_create(&res->thread_write, NULL, at_queue_thread_write, res);

    return(res);
}

/*------------------------------------------------------------------------*/

void at_queue_destroy(at_queue_t* priv)
{
    void* thread_res;

	if(!priv)
		return;

    priv->terminate = 1;

    pthread_join(priv->thread_write, &thread_res);
	pthread_join(priv->thread_read, &thread_res);

    /* cleanup used resources */
    close(priv->fd);

    queue_destroy(priv->q);

    pthread_mutex_destroy(&priv->mutex);
    pthread_cond_destroy(&priv->processed);

    free(priv);
}

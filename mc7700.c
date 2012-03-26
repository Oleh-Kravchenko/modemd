#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>

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

	printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

	while(!priv->terminate)
 	{
		/* receive pointer to query */
		if(queue_wait_pop(priv->q, THREAD_WAIT, &buf, &buf_len))
			continue;

		if(buf_len != sizeof(void**))
			continue;

		/* resolve pointer to query */
		query = (mc7700_query_t*)*((mc7700_query_t**)buf);
		
		printf("%s:%d %s() query->query = %s\n", __FILE__, __LINE__, __func__, query->query);

		write(priv->fd, query->query, strlen(query->query));

		/* wait for answer */
		pthread_mutex_lock(&mutex_mc7700);
		pthread_cond_wait(&priv->processed, &mutex_mc7700);
		pthread_mutex_unlock(&mutex_mc7700);

		printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);
	}

	return(NULL);
}

/*------------------------------------------------------------------------*/

void* mc7700_thread_read(void* prm)
{
    thread_queue_t *priv = prm;
    struct pollfd p = {priv->fd, POLLIN, 0};
    char buf[0xffff];
    int buf_len, i, res;

    while(!priv->terminate)
    {
		printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

		res = poll(&p, 1, THREAD_WAIT * 1000);

        if(res > 0 && p.revents & POLLIN)
        {
            buf_len = read(priv->fd, buf, sizeof(buf) - 1);

            if(buf_len > 0)
            {
				buf[buf_len] = 0;
                query->answer = malloc(buf_len + 1);
                strncpy(query->answer, buf, buf_len + 1);

				printf("%s:%d %s() query->answer = %s\n", __FILE__, __LINE__, __func__, query->answer);
            }
        }
        else
		{
			if(query)
			{
				printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

				pthread_mutex_lock(&query->cond_m);
				pthread_cond_signal(&query->cond);
				pthread_mutex_unlock(&query->cond_m);
				query = NULL;
			}

			printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

			/* notify writing thread */
            pthread_mutex_lock(&mutex_mc7700);
            pthread_cond_signal(&priv->processed);
            pthread_mutex_unlock(&mutex_mc7700);
		}
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

mc7700_query_t* mc7700_query_create(const char* query, const char* answer_reg)
{
	mc7700_query_t* res;

	if(!(res = malloc(sizeof(*res))))
		goto err;

	if(!(res->query = malloc(strlen(query) + 1)))
		goto err_q;

	strncpy(res->query, query, strlen(query) + 1);

	if(!(res->answer_reg = malloc(strlen(answer_reg) + 1)))
		goto err_a;

	strncpy(res->answer_reg, answer_reg, strlen(answer_reg) + 1);

	res->answer = NULL;

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

int mc7700_query_proccess(queue_t* q, mc7700_query_t* query)
{
	int res;

	if((res = queue_add(q, &query, sizeof(mc7700_query_t**))))
		goto err;

	printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

	/* wait for processing */
	pthread_mutex_lock(&query->cond_m);
	pthread_cond_wait(&query->cond, &query->cond_m);
	pthread_mutex_unlock(&query->cond_m);

	printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

err:
	return(res);
}

/*------------------------------------------------------------------------*/

void mc7700_query_destroy(mc7700_query_t* query)
{
	free(query->answer);
	free(query->answer_reg);
	free(query->query);
	pthread_cond_destroy(&query->cond);
	pthread_mutex_destroy(&query->cond_m);
	free(query);
}

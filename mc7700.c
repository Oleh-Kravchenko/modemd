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

/*------------------------------------------------------------------------*/

static int mc7700_clients = 0;

static pthread_t thread_write, thread_read;

thread_queue_t thread_priv;

static pthread_mutex_t mutex_mc7700 = PTHREAD_MUTEX_INITIALIZER;

/*------------------------------------------------------------------------*/

void* mc7700_thread_write(void* prm)
{
    thread_queue_t *priv = prm;
	char* s;
	size_t size;

	printf("==== %s:%d %s()\n", __FILE__, __LINE__, __func__);

	while(!priv->terminate)
 	{
		if(queue_pop(priv->q, &s, &size))
		{
			sleep(1);
			continue;
		}

		printf("==== %s:%d %s()\n", __FILE__, __LINE__, __func__);

		write(priv->fd, s, size);
		free(s);

		pthread_cond_wait(&priv->cond, &mutex_mc7700);
	}

	printf("==== %s:%d %s()\n", __FILE__, __LINE__, __func__);
}

/*------------------------------------------------------------------------*/

void* mc7700_thread_read(void* prm)
{
    thread_queue_t *priv = prm;
    struct pollfd p = {0, POLLIN, 0};
    int res;
    char buf[0xffff];
    int buf_len, i;

    p.fd = priv->fd;

	printf("==== %s:%d %s()\n", __FILE__, __LINE__, __func__);

    while(!priv->terminate)
    {
        res = poll(&p, 1, 1000);

        if(res > 0 && p.revents & POLLIN)
        {
            buf_len = read(priv->fd, buf, sizeof(buf));

            if (buf_len > 0)
            {
                printf("READ: ");
                for (i = 0; i < buf_len; ++ i)
                    printf("%c", buf[i]);
            }
        }
        else
            pthread_cond_signal(&priv->cond);
    }
}

/*------------------------------------------------------------------------*/

int mc7700_open(const char *port)
{
	if(mc7700_clients ++)
		return;

	thread_priv.fd = serial_open(port, O_RDWR);
	thread_priv.q = queue_create();
	thread_priv.terminate = 0;
	pthread_cond_init(&thread_priv.cond, NULL);

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

	if(mc7700_clients)
	{
		-- mc7700_clients;
	}
	else
	{
		thread_priv.terminate = 1;

		pthread_join(thread_write, &thread_res);
		pthread_join(thread_read, &thread_res);

		pthread_cond_destroy(&thread_priv.cond);
	}
}

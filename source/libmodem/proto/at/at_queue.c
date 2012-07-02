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

#include "modem/modem_errno.h"

#include "queue.h"
#include "at_queue.h"
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
		if(queue_wait_pop(priv->queue, THREAD_WAIT, &buf, &buf_len))
			continue;

		if(buf_len != sizeof(void**))
		{
			free(buf);
			continue;
		}

		/* resolve pointer to query */
		priv->query = (at_query_t*)*((at_query_t**)buf);
		free(buf);

//		printf("(EE) write(%zd): [%s]\n", strlen(priv->query->cmd), priv->query->cmd);

		syslog(LOG_INFO | LOG_LOCAL7, "write() [%s]", priv->query->cmd);

		if(write(priv->fd, priv->query->cmd, strlen(priv->query->cmd)) == -1)
		{
			priv->last_error = priv->query->error = __ME_WRITE_FAILED; /* unknown error */

			/* reporting about failed command */
			event_signal(priv->query->event);

			priv->query = NULL;

			continue;
		}

		/* wait for answer */
		event_wait(priv->event);
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

//				printf("(EE) read(%d): %s", buf_len, buf);
				syslog(LOG_INFO | LOG_LOCAL7, "read() [%s]", buf);

				regcomp(&re, priv->query->re_res, REG_EXTENDED);
				priv->query->nmatch = re.re_nsub + 1;
				priv->query->pmatch = malloc(sizeof(regmatch_t) * priv->query->nmatch);
				re_res = regexec(&re, buf, priv->query->nmatch, priv->query->pmatch, 0);

				if(re_res)
				{
					char re_err[0x100];

					regerror(re_res, &re, re_err, sizeof(re_err));

//					printf("(EE) regexec() %s [\n%s\n]\n", re_err, buf);

					free(priv->query->pmatch);
					priv->query->pmatch = NULL;
					regfree(&re);

					priv->query->error = at_parse_error(buf);

					if(priv->query->error == -1)
						/* no error detected, proceed collecting data */
						continue;
				}

				regfree(&re);
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
//				printf("(EE) Command %s expired after %d second(s)\n", priv->query->cmd, giveup);
//				printf("(EE) No match %s\n", buf);
				priv->last_error = priv->query->error = __ME_READ_FAILED;
			}
			else //if(priv->query->error > 0) /* ignore general error */
			{
//				printf("(EE) CME ERROR: %d %p\n", priv->query->error, (void*)priv);
				priv->last_error = priv->query->error;
			}

//			printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

			priv->query->result = malloc(buf_len + 1);
			strncpy(priv->query->result, buf, buf_len );
			priv->query->result[buf_len] = 0;

			re_res = -1;
			
			/* reporting about reply */
			event_signal(priv->query->event);

			buf_len = 0;
			giveup = 0;
			priv->query = NULL;

			/* notify writing thread */
			event_signal(priv->event);
		}
		else if(priv->query)
			++ giveup;
	}

	return(NULL);
}

/*------------------------------------------------------------------------*/

at_queue_t* at_queue_open(const char *dev)
{
	at_queue_t *res;

	if((res = malloc(sizeof(*res))) == NULL)
		return(res);

	res->fd = serial_open(dev, O_RDWR);
	res->queue = queue_create();
	res->terminate = 0;
	res->query = NULL;
	res->last_error = -1;

	res->event = event_create();

	/* creating reading thread */
	pthread_create(&res->thread_read, NULL, at_queue_thread_read, res);

	/* creating write thread */
	pthread_create(&res->thread_write, NULL, at_queue_thread_write, res);

	return(res);
}

/*------------------------------------------------------------------------*/

void at_queue_destroy(at_queue_t* at_queue)
{
	void* thread_res;

	if(!at_queue)
		return;

	at_queue->terminate = 1;

	pthread_join(at_queue->thread_write, &thread_res);
	pthread_join(at_queue->thread_read, &thread_res);

	/* cleanup used resources */
	close(at_queue->fd);

	queue_destroy(at_queue->queue);
	event_destroy(at_queue->event);

	free(at_queue);
}

/*------------------------------------------------------------------------*/

void at_queue_suspend(at_queue_t* at_queue)
{
	void* thread_res;

	if(!at_queue)
		return;

	at_queue->terminate = 1;

	pthread_join(at_queue->thread_write, &thread_res);
	pthread_join(at_queue->thread_read, &thread_res);

	/* cleanup used resources */
	close(at_queue->fd);

	at_queue->fd = -1;
}

/*------------------------------------------------------------------------*/

void at_queue_resume(at_queue_t* at_queue, const char *dev)
{
	if(!at_queue || at_queue->fd > -1)
		return;

	at_queue->fd = serial_open(dev, O_RDWR);
	at_queue->terminate = 0;
	at_queue->last_error = -1;

	/* creating reading thread */
	pthread_create(&at_queue->thread_read, NULL, at_queue_thread_read, at_queue);

	/* creating write thread */
	pthread_create(&at_queue->thread_write, NULL, at_queue_thread_write, at_queue);
}

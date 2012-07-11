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
#include <syslog.h>

#include "modem/modem_errno.h"

#include "queue.h"

#include "at_queue.h"
#include "at/at_utils.h"
#include "at/at_common.h"

#include "utils/re.h"
#include "utils/str.h"
#include "utils/file.h"

/*------------------------------------------------------------------------*/

#define THREAD_WAIT 1

/*------------------------------------------------------------------------*/

void* at_queue_thread_write(void* prm)
{
	at_queue_t* at_q = prm;
	at_query_t* q;
	void* buf = NULL;
	size_t buf_len;

	while(!at_q->terminate)
	{
		/* receive pointer to query */
		if(queue_wait_pop(at_q->queue, THREAD_WAIT, &buf, &buf_len))
			continue;

		if(buf_len != sizeof(void**))
		{
			free(buf);
			continue;
		}

		/* resolve pointer to query */
		q = (at_query_t*)*((at_query_t**)buf);

		free(buf);

		syslog(LOG_INFO | LOG_LOCAL7, "write() [%s]", q->cmd);

		if(write(at_q->fd, q->cmd, strlen(q->cmd)) == -1)
		{
			/* failed to write command */
			q->error = at_q->last_error = __ME_WRITE_FAILED;

			/* reporting about failed command */
			event_signal(at_q->event);

			continue;
		}

		at_q->query = q;

		/* wait for answer */
		event_wait(at_q->event);
	}

	return(NULL);
}

/*------------------------------------------------------------------------*/

void* at_queue_thread_read(void* prm)
{
	at_queue_t* at_q = prm;
	struct pollfd p;
	char buf[0x10000];
	int buf_len = 0;
	int timeout = 0;
	int res = 1;

	while(!at_q->terminate)
	{
		if(at_q->query)
		{
			++ timeout;

			/* check timeout or for read error */
			if(timeout > at_q->query->timeout || res <= 0)
			{
				at_q->query->error = __ME_READ_FAILED;

				goto query_done;
			}
		}

		/* filling pollfd */
		p.fd = at_q->fd;
		p.events = POLLIN;
		p.revents = 0;

		/* wait for input data */
		if(!(poll(&p, 1, THREAD_WAIT * 1000) > 0 && p.revents & POLLIN))
			continue;

		/* reading data */
		if((res = read(at_q->fd, buf + buf_len, sizeof(buf) - buf_len - 1)) <= 0)
			continue;

		/* if query not set, this is unsolicited response */
		if(!at_q->query)
		{
			buf_len = 0;

			continue;
		}

		/* accumulating data */
		buf_len += res;
		buf[buf_len] = 0;

		syslog(LOG_INFO | LOG_LOCAL7, "read() [%s]", buf);

		/* compare text with regular expression */
		if(re_parse(buf, at_q->query->re_res, &at_q->query->nmatch, &at_q->query->pmatch))
		{
			at_q->query->pmatch = NULL;

			/* no error detected, proceed collecting data */
			if((at_q->query->error = at_parse_error(buf)) == -1)
				continue;
		}

		/* saving buf as answer */
		at_q->query->result = malloc(buf_len + 1);
		strncpy(at_q->query->result, buf, buf_len);
		at_q->query->result[buf_len] = 0;

		query_done:
		{
			at_q->last_error = at_q->query->error;

			/* reporting about reply */
			event_signal(at_q->query->event);

			/*printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);*/
			at_q->query = NULL;

			buf_len = 0;
			timeout = 0;

			/* notify writing thread for next command */
			event_signal(at_q->event);
		}
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

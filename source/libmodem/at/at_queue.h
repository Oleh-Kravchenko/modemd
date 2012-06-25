#ifndef __AT_QUEUE_H
#define __AT_QUEUE_H

#include <regex.h>

#include "at/at_query.h"
#include "modem/types.h"
#include "../libmodem/queue.h"
#include "utils/event.h"
 
/*------------------------------------------------------------------------*/

typedef struct
{
	int fd;

	event_t* event;

	queue_t *queue;

	at_query_t* query;

	int terminate;

	pthread_t thread_write;

	pthread_t thread_read;

	int32_t last_error;
} at_queue_t;

/*------------------------------------------------------------------------*/

at_queue_t* at_queue_open(const char *dev);

void at_queue_destroy(at_queue_t* at_queue);

void at_queue_suspend(at_queue_t* at_queue);

void at_queue_resume(at_queue_t* at_queue, const char *dev);

#endif /* __AT_QUEUE_H */

#ifndef __QUEUE_H
#define __QUEUE_H

#include <stdlib.h>
#include <string.h>

#include "utils/event.h"

/*------------------------------------------------------------------------*/

typedef struct queue_item_s
{
	/** pointer to data */
	void *data;

	/** size of data */
	size_t size;

	/** pointer to next item */
	struct queue_item_s *next;
} queue_item_t;

/*------------------------------------------------------------------------*/

typedef struct {
	/** pointer to first item of queue */
	queue_item_t *first;

	/** pointer to last item of queue */
	queue_item_t *last;

	/** queue busy flag for adding */
	int busy;

	/** mutex for queue managment */
	pthread_mutex_t lock;

	event_t* event;
} queue_t;

/*------------------------------------------------------------------------*/

/**
 * @brief create queue
 * @return pointer to queue
 *
 * Queue must be destroyed by function queue_destroy()
 */
queue_t* queue_create(void);

/**
 * @brief destroy queue
 * @param q pointer to queue
 */
void queue_destroy(queue_t* q);

/**
 * @brief add data to the queue
 * @param q queue
 * @param data pointer to the data
 * @param size size of data
 * @return 0 if successful
 */
int queue_add(queue_t* q, const void* data, size_t size);

/**
 * @brief pop data from the queue
 * @param q queue
 * @param data pointer to data
 * @param size size of data
 * @return 0 if successful
 *
 * data must be freed by function free()
 */
int queue_pop(queue_t* q, void** data, size_t* size);

/**
 * @brief wait to pop data from the queue
 * @param q queue
 * @param sec timeout in seconds before give up
 * @param data pointer to data
 * @param size size of data
 * @return 0 if successful
 *
 * data must be freed by function free()
 */
int queue_wait_pop(queue_t* q, int seconds, void** data, size_t* size);

/**
 * @brief setup busy flag of queue
 * @param q queue
 * @param busy busy flag
 * @return old value of flag
 */
int queue_busy(queue_t* q, int busy);

#endif /* __QUEUE_H */

#ifndef __QUEUE_H
#define __QUEUE_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct queue_item_s {
    void *data;

    size_t size;

    struct queue_item_s *next;
} queue_item_t;

/*------------------------------------------------------------------------*/

typedef struct {
	/* pointer to first item of queue */
    queue_item_t *first;

	/* pointer to last item of queue */
    queue_item_t *last;

    /** mutex for queue managment */
    pthread_mutex_t lock;

	/** mutex for wait condition queue population */
    pthread_mutex_t cond_lock;

	/** condition for wait queue population */
	pthread_cond_t cond;
} queue_t;

/*------------------------------------------------------------------------*/

/**
 * @brief create queue
 * @return pointer to queue
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
 **/
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
 * @brief pop data from the queue
 * @param q queue
 * @param sec timeout in seconds before give up
 * @param data pointer to data
 * @param size size of data
 * @return 0 if successful
 *
 * data must be freed by function free()
 */
int queue_wait_pop(queue_t* q, int seconds, void** data, size_t* size);

#endif /* __QUEUE_H */

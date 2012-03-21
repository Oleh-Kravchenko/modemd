#ifndef __QUEUE_H
#define __QUEUE_H

#include <pthread.h>

typedef struct queue_item_s {
    char *data;

    size_t size;

    struct queue_item_s *next;
} queue_item_t;

/*------------------------------------------------------------------------*/

typedef struct {
    queue_item_t *first;

    queue_item_t *last;

    pthread_mutex_t mutex;
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
int queue_add(queue_t* q, const char* data, size_t size);

/**
 * @brief pop data from the queue
 * @param q queue
 * @param data pointer to data
 * @param size size of data
 * @return 0 if successful
 *
 * data must be freed by function free()
 */
int queue_pop(queue_t* q, char** data, size_t* size);

#endif /* __QUEUE_H */

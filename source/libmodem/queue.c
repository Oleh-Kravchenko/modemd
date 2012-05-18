#include <stdio.h>

#include "queue.h"

/*------------------------------------------------------------------------*/

queue_t* queue_create(void)
{
    queue_t* res;

    if((res = malloc(sizeof(*res))))
    {
        res->first = NULL;
        res->last = NULL;
        res->busy = 0;

        pthread_mutex_init(&res->lock, NULL);
        pthread_mutex_init(&res->cond_lock, NULL);
        pthread_cond_init(&res->cond, NULL);
    }

    return(res);
}

/*------------------------------------------------------------------------*/

void queue_destroy(queue_t* q)
{
    queue_item_t *j, *i;

    if(!q)
        /* if NULL nothing to do */
        return;

    i = q->first;

    while (i)
    {
        j = i;
        i = i->next;

        free(j->data);
        free(j);
    }

    pthread_mutex_destroy(&q->lock);
    pthread_mutex_destroy(&q->cond_lock);
    pthread_cond_destroy(&q->cond);

    free(q);
}

/*------------------------------------------------------------------------*/

int queue_add(queue_t* q, const void* data, size_t size)
{
    queue_item_t *i;
    int busy;

    /* check queue busy state */
    pthread_mutex_lock(&q->lock);
    busy = q->busy;
    pthread_mutex_unlock(&q->lock);

    /* create item if not a busy */
    if(busy || !(i = malloc(sizeof(*i))))
        return(-1);

    if(!(i->data = malloc(size)))
    {
        free(i);

        return(-1);
    }

    /* filling item data */
    memcpy(i->data, data, size);
    i->size = size;
    i->next = NULL;

    pthread_mutex_lock(&q->lock);

    /* add item to the list */
    if(q->first)
    {
        q->last->next = i;
        q->last = i;
    }
    else
    {
        q->first = i;
        q->last = i;
    }

    /* notify about new item */
    pthread_mutex_lock(&q->cond_lock);
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->cond_lock);

    pthread_mutex_unlock(&q->lock);

    return(0);
}

/*------------------------------------------------------------------------*/

int queue_pop(queue_t* q, void** data, size_t* size)
{
    queue_item_t *i;
    int res = 0;

    pthread_mutex_lock(&q->lock);

    if(!(i = q->first))
    {
        res = -1;
        goto err;
    }

    *data = i->data;
    *size = i->size;

    /* pop item from the list */
    q->first = i->next;

    /* if list contain only one item */
    if(i == q->last)
        q->last = NULL;

    free(i);

err:
    pthread_mutex_unlock(&q->lock);

    return(res);
}

/*------------------------------------------------------------------------*/

int queue_wait_pop(queue_t* q, int seconds, void** data, size_t* size)
{
    struct timespec timeout;
    int res, mutex_res;

    /* try pop message */
    while((res = queue_pop(q, data, size)))
    {
        /* setup timeout */
        timeout.tv_sec = time(NULL) + seconds;
        timeout.tv_nsec = 0;

        /* pop failed wait sec and try it again */
        pthread_mutex_lock(&q->cond_lock);
        mutex_res = pthread_cond_timedwait(&q->cond, &q->cond_lock, &timeout);
        pthread_mutex_unlock(&q->cond_lock);

        if(mutex_res)
            /* if timeout exit */
            break;
    }

    return(res);
}

/*------------------------------------------------------------------------*/

int queue_busy(queue_t* q, int busy)
{
    pthread_mutex_lock(&q->lock);

    /* swaping values q->busy <=> busy */
    q->busy += busy;
    busy = q->busy - busy;
    q->busy -= busy;

    pthread_mutex_unlock(&q->lock);

    return(busy);
}

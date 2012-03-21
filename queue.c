#include <stdlib.h>
#include <string.h>

#include "queue.h"

/*------------------------------------------------------------------------*/

queue_t* queue_create(void)
{
    queue_t* res;

    if ((res = malloc(sizeof(*res))))
    {
        res->first = NULL;
        res->last = NULL;

		pthread_mutex_init(&res->mutex, NULL);
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

    pthread_mutex_destroy(&q->mutex);

    free(q);
}

/*------------------------------------------------------------------------*/

int queue_add(queue_t* q, const char* data, size_t size)
{
    queue_item_t *i;
	int res = 0;

	/* create item */
    if(!(i = malloc(sizeof(*i))))
		goto err;

    if(!(i->data = malloc(size)))
		goto err_data;

    memcpy(i->data, data, size);
    i->size = size;
	i->next = NULL;

	pthread_mutex_lock(&q->mutex);

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

	pthread_mutex_unlock(&q->mutex);

    goto exit;

err_data:
	free(i);

err:
	res = -1;

exit:
	return(res);
}

/*------------------------------------------------------------------------*/

int queue_pop(queue_t* q, char** data, size_t* size)
{
    queue_item_t *i;
	int res = 0;

	pthread_mutex_lock(&q->mutex);

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
	pthread_mutex_unlock(&q->mutex);

	return(res);
}

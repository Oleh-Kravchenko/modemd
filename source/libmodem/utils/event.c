#include <time.h>
#include <stdlib.h>

#include "event.h"

/*------------------------------------------------------------------------*/

event_t* event_create(void)
{
	event_t* res;

	if(!(res = malloc(sizeof(*res))))
		return(res);

	pthread_cond_init(&res->cond, NULL);
	pthread_mutex_init(&res->mutex, NULL);

	return(res);
}

/*------------------------------------------------------------------------*/

void event_wait(event_t* event)
{
	pthread_mutex_lock(&event->mutex);
	pthread_cond_wait(&event->cond, &event->mutex);
	pthread_mutex_unlock(&event->mutex);
}

/*------------------------------------------------------------------------*/

int event_wait_time(event_t* event, int seconds)
{
	struct timespec timeout;
	int res;

	/* setup timeout */
	timeout.tv_sec = time(NULL) + seconds;
	timeout.tv_nsec = 0;

	pthread_mutex_lock(&event->mutex);
	res = pthread_cond_timedwait(&event->cond, &event->mutex, &timeout);
	pthread_mutex_unlock(&event->mutex);

	return(res);
}

/*------------------------------------------------------------------------*/

void event_signal(event_t* event)
{
	pthread_mutex_lock(&event->mutex);
	pthread_cond_signal(&event->cond);
	pthread_mutex_unlock(&event->mutex);
}

/*------------------------------------------------------------------------*/

void event_destroy(event_t* event)
{
	if(!event)
		return;

	pthread_cond_destroy(&event->cond);
	pthread_mutex_destroy(&event->mutex);

	free(event);
}

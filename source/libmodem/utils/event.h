#ifndef __EVENT_H
#define __EVENT_H

#include <pthread.h>

/*------------------------------------------------------------------------*/

typedef struct
{
	pthread_cond_t cond;

	pthread_mutex_t mutex;
} event_t;

/*------------------------------------------------------------------------*/

event_t* event_create(void);

void event_wait(event_t* event);

int event_wait_time(event_t* event, int seconds);

void event_signal(event_t* event);

void event_signal_all(event_t* event);

void event_destroy(event_t* event);

#endif /* __EVENT_H */

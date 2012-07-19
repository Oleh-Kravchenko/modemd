#ifndef __QCQMI_QUEUE_H
#define __QCQMI_QUEUE_H

#include <pthread.h>

#include "modem/types.h"

/*------------------------------------------------------------------------*/

typedef struct
{
	int terminate;

	unsigned long last_error;

	pthread_mutex_t mutex;
} qcqmi_queue_t;

/*------------------------------------------------------------------------*/

qcqmi_queue_t* qcqmi_queue_open(const char* dev, const char* imei);

void qcqmi_queue_destroy(qcqmi_queue_t* queue);

void qcqmi_queue_suspend(qcqmi_queue_t* queue);

void qcqmi_queue_resume(qcqmi_queue_t* queue, const char* dev, const char* imei);

#endif /* __QCQMI_QUEUE_H */

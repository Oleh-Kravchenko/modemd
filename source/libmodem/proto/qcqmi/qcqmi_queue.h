#ifndef __QCQMI_QUEUE_H
#define __QCQMI_QUEUE_H

#include "modem/types.h"

/*------------------------------------------------------------------------*/

typedef struct
{
	int terminate;
} qcqmi_queue_t;

/*------------------------------------------------------------------------*/

qcqmi_queue_t* qcqmi_queue_open(const char* dev);

void qcqmi_queue_destroy(qcqmi_queue_t* queue);

void qcqmi_queue_suspend(qcqmi_queue_t* queue);

void qcqmi_queue_resume(qcqmi_queue_t* queue, const char* dev);

#endif /* __QCQMI_QUEUE_H */

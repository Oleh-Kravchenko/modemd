#include <stdio.h>
#include <stdlib.h>

#include "qcqmi/qcqmi_queue.h"

/*------------------------------------------------------------------------*/

qcqmi_queue_t* qcqmi_queue_open(const char* dev)
{
	printf("(WW) Not implemented %s()\n", __func__);

	return(malloc(sizeof(qcqmi_queue_t)));
}

/*------------------------------------------------------------------------*/

void qcqmi_queue_destroy(qcqmi_queue_t* queue)
{
	if(!queue)
		return;

	printf("(WW) Not implemented %s()\n", __func__);

	free(queue);
}

/*------------------------------------------------------------------------*/

void qcqmi_queue_suspend(qcqmi_queue_t* queue)
{
	printf("(WW) Not implemented %s()\n", __func__);
}

/*------------------------------------------------------------------------*/

void qcqmi_queue_resume(qcqmi_queue_t* queue, const char* dev)
{
	printf("(WW) Not implemented %s()\n", __func__);
}

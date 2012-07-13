#include <stdio.h>
#include <stdlib.h>

#include <SWIWWANCMAPI.h>
#include <qmerrno.h>

#include "qcqmi/qcqmi_queue.h"

/*------------------------------------------------------------------------*/

qcqmi_queue_t* qcqmi_queue_open(const char* dev, const char* imei)
{
	qcqmi_queue_t* res;

	if(!(res = malloc(sizeof(*res))))
		return(res);

	res->terminate = 0;
	res->last_error = QCWWANConnect(dev, imei);

	printf("(II) QCWWAN2kConnect(%s, %s) = %d\n", dev, imei, res->last_error);

	if(res->last_error != eQCWWAN_ERR_NONE)
	{
		free(res);

		res = NULL;
	}

	return(res);
}

/*------------------------------------------------------------------------*/

void qcqmi_queue_destroy(qcqmi_queue_t* queue)
{
	if(!queue)
		return;

	QCWWANDisconnect();

	free(queue);
}

/*------------------------------------------------------------------------*/

void qcqmi_queue_suspend(qcqmi_queue_t* queue)
{
	printf("(WW) Not implemented %s()\n", __func__);
}

/*------------------------------------------------------------------------*/

void qcqmi_queue_resume(qcqmi_queue_t* queue, const char* dev, const char* imei)
{
	printf("(WW) Not implemented %s()\n", __func__);
}

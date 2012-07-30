#include <stdio.h>
#include <stdlib.h>

#include <SWIWWANCMAPI.h>
#include <qmerrno.h>

#include "qcqmi/qcqmi_queue.h"

/*------------------------------------------------------------------------*/

void CallBackSLQSSignalStrengths(struct SLQSSignalStrengthsInformation sssi)
{
	printf("(CB) %s() signal strench = %d dBm\n", __func__, sssi.rxSignalStrengthInfo.rxSignalStrength);
}

/*------------------------------------------------------------------------*/

qcqmi_queue_t* qcqmi_queue_open(const char* dev, const char* imei)
{
	struct SLQSSignalStrengthsIndReq sssiq = {
		.rxSignalStrengthDelta = 1,
		.ecioDelta = 1,
		.ioDelta = 1,
		.sinrDelta = 1,
		.rsrqDelta = 1,
		.ecioThresholdListLen = 5,
		.ecioThresholdList = {-10, -20, -30, -40, -50,},
		.sinrThresholdListLen = 5,
		.sinrThresholdList = {10, 20, 30, 40, 50,},
	};
	qcqmi_queue_t* res;

	if(!(res = malloc(sizeof(*res))))
		return(res);

	memset(res, 0, sizeof(*res));

	printf("(II) QCWWAN2kConnect(%s, %s) = %d\n",
		dev, imei, res->last_error = QCWWANConnect((CHAR*)dev, (CHAR*)imei));

	if(res->last_error != eQCWWAN_ERR_NONE)
	{
		free(res);

		res = NULL;

		return(res);
	}

	pthread_mutex_init(&res->mutex, NULL);

	printf("(II) SLQSSetSignalStrengthsCallback() = %d\n", SLQSSetSignalStrengthsCallback(CallBackSLQSSignalStrengths, &sssiq));

	return(res);
}

/*------------------------------------------------------------------------*/

void qcqmi_queue_destroy(qcqmi_queue_t* queue)
{
	if(!queue)
		return;

	SLQSSetSignalStrengthsCallback(NULL, NULL);

	QCWWANDisconnect();

	pthread_mutex_destroy(&queue->mutex);

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

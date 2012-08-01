#include <stdio.h>
#include <stdlib.h>

#include <SWIWWANCMAPI.h>
#include <qmerrno.h>

#include "qcqmi/qcqmi_queue.h"

/*------------------------------------------------------------------------*/

typedef struct device_info_s
{
	uint8_t dev[256];

	uint8_t key[16];
} device_info_t;

/*------------------------------------------------------------------------*/

qcqmi_queue_t* qcqmi_queue_open(const char* dev)
{
	qcqmi_queue_t* res;
	uint8_t key[16] = {0};
	device_info_t devices[10];
	uint8_t i, n_devices = ARRAY_SIZE(devices);

	if(!(res = malloc(sizeof(*res))))
		return(res);

	memset(res, 0, sizeof(*res));

	printf("(II) QCWWAN2kEnumerateDevices() = %d\n",
		res->last_error = QCWWAN2kEnumerateDevices(&n_devices, devices));

	if(res->last_error != eQCWWAN_ERR_NONE)
		goto err;

	/* let's find our modem key by dev path */
	for(i = 0; i < n_devices; ++ i)
	{
		if(!strcmp(devices[i].dev, dev))
		{
			printf("\tNode: %s, Key: %s\n", devices[i].dev, devices[i].key);

			strncpy(key, devices[i].key, sizeof(key));
			key[sizeof(key) - 1] = 0;

			break;
		}
	}

	/* if modem key is invalid, then error */
	if(!*key)
		goto err;

	printf("(II) QCWWAN2kConnect(%s, %s) = %d\n",
		dev, key, res->last_error = QCWWAN2kConnect(dev, key));

	if(res->last_error != eQCWWAN_ERR_NONE)
		goto err;

	pthread_mutex_init(&res->mutex, NULL);

	return(res);

err:
	free(res);

	return(NULL);
}

/*------------------------------------------------------------------------*/

void qcqmi_queue_destroy(qcqmi_queue_t* queue)
{
	if(!queue)
		return;

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

void qcqmi_queue_resume(qcqmi_queue_t* queue, const char* dev)
{
	printf("(WW) Not implemented %s()\n", __func__);
}

#include <stdio.h>

#include <modem/modem_str.h>

#include "modem_db.h"
#include "proto.h"

#include "utils/sysfs.h"

#include "at/at_queue.h"
#include "at/at_common.h"

#ifdef __QCQMI
#	include "qcqmi/qcqmi_queue.h"
#endif /* __QCQMI */

/*------------------------------------------------------------------------*/

int modem_queues_init(modem_t* modem)
{
	const modem_db_device_t* mdd = modem->mdd;
	char dev[0x100], imei[0x100];
	void* queue;
	int i;

	for(i = 0; mdd->iface[i].type && i < ARRAY_SIZE(mdd->iface); ++ i)
	{
		switch(mdd->iface[i].type)
		{
			case MODEM_PROTO_AT:
				if(!modem_get_iface_dev(modem->port, "tty", mdd->iface[i].num, dev, sizeof(dev)))
				{
					printf("(EE) Failed modem_get_iface_dev(%d)..\n", mdd->iface[i].num);

					goto err;
				}

				if(!(queue = at_queue_open(dev)))
				{
					printf("(EE) Failed at_queue_open()..\n");

					goto err;
				}

				if(modem_queues_add(modem, mdd->iface[i].type, queue))
				{
					at_queue_destroy(queue);

					printf("(EE) Failed MODEM_PROTO_AT..\n");

					goto err;
				}

				break;

#ifdef __QCQMI
			case MODEM_PROTO_QCQMI:
				at_get_imei(modem, imei, sizeof(imei));

				if(!modem_get_iface_dev(modem->port, "qcqmi", mdd->iface[i].num, dev, sizeof(dev)))
				{
					printf("(EE) Failed modem_get_iface_dev(%d)..\n", mdd->iface[i].num);

					goto err;
				}

				if(!(queue = qcqmi_queue_open(dev, imei)))
				{
					printf("(EE) Failed qcqmi_queue_open()..\n");
					goto err;
				}

				if(modem_queues_add(modem, mdd->iface[i].type, queue))
				{
					qcqmi_queue_destroy(queue);

					printf("(EE) Failed MODEM_PROTO_QCQMI..\n");

					goto err;
				}

				break;
#endif /* __QCQMI */

			default:
				printf("(EE) Driver not loaded or old firmware..\n");

				goto err;
		}
	}

	return(0);

err:
	modem_queues_destroy(modem);

	return(1);
}

/*------------------------------------------------------------------------*/

void modem_queues_destroy(modem_t* modem)
{
	modem_queues_t* mq;

	while((mq = modem->queues))
	{
		switch(mq->proto)
		{
			case MODEM_PROTO_AT:
				at_queue_destroy(mq->queue);
				break;

#ifdef __QCQMI
			case MODEM_PROTO_QCQMI:
				qcqmi_queue_destroy(mq->queue);
				break;
#endif /* __QCQMI */

			default:
				printf("(EE) %s() queue %s not implemented\n", __func__, str_proto(mq->proto));
				break;
		}

		modem->queues = mq->next;

		free(mq);
	}
}

/*------------------------------------------------------------------------*/

void modem_queues_suspend(modem_t* modem)
{
	modem_queues_t* mq = modem->queues;

	while(mq)
	{
		switch(mq->proto)
		{
			case MODEM_PROTO_AT:
				at_queue_suspend(mq->queue);
				break;

#ifdef __QCQMI
			case MODEM_PROTO_QCQMI:
				qcqmi_queue_suspend(mq->queue);
				break;
#endif /* __QCQMI */

			default:
				printf("(EE) %s() queue %s not implemented\n", __func__, str_proto(mq->proto));
				break;
		}

		mq = mq->next;
	}
}

/*------------------------------------------------------------------------*/

void modem_queues_resume(modem_t* modem)
{
	const modem_db_device_t* mdd = modem->mdd;
	modem_queues_t* mq = modem->queues;
	char dev[0x100], imei[0x100];
	int i;

	while(mq)
	{
		for(i = 0; i < __MODEM_IFACE_MAX; ++ i)
			if(mdd->iface[i].type == mq->proto)
				break;

		if(i == __MODEM_IFACE_MAX)
		{
			printf("(EE) %s:%d %s()\n", __FILE__, __LINE__, __func__);
			continue;
		}

		switch(mq->proto)
		{
			case MODEM_PROTO_AT:
				modem_get_iface_dev(modem->port, "tty", mdd->iface[i].num, dev, sizeof(dev));
				at_queue_resume(mq->queue, dev);
				break;

#ifdef __QCQMI
			case MODEM_PROTO_QCQMI:
				at_get_imei(modem, imei, sizeof(imei));
				modem_get_iface_dev(modem->port, "qcqmi", mdd->iface[i].num, dev, sizeof(dev));
				qcqmi_queue_resume(mq->queue, dev, imei);
				break;
#endif /* __QCQMI */

			default:
				printf("(EE) %s() queue %s not implemented\n", __func__, str_proto(mq->proto));
				break;
		}

		mq = mq->next;
	}
}

/*------------------------------------------------------------------------*/

void* modem_proto_get(modem_t* modem, modem_proto_t proto)
{
	modem_queues_t* mq = modem->queues;

	while(mq)
	{
		if(mq->proto == proto)
			return(mq->queue);

		mq = mq->next;
	}

	return(NULL);
}

/*------------------------------------------------------------------------*/

int modem_queues_add(modem_t* modem, modem_proto_t proto, void* queue)
{
	modem_queues_t* item;

	if(!(item = malloc(sizeof(*item))))
		return(-1);

	/* queue item */
	item->proto = proto;
	item->queue = queue;
	item->next = modem->queues;

	/* inserting new queue */
	modem->queues = item;

	return(0);
}

/*------------------------------------------------------------------------*/

int modem_queues_last_error(modem_t* modem, modem_proto_t proto)
{
	modem_queues_t* mq;
	int res = -1;

	for(mq = modem->queues; mq; mq = mq->next)
	{
		if(mq->proto != proto)
			continue;

		switch(mq->proto)
		{
			case MODEM_PROTO_AT:
				res = ((at_queue_t*)mq->queue)->last_error;
				break;

#ifdef __QCQMI
			case MODEM_PROTO_QCQMI:
				res = ((qcqmi_queue_t*)mq->queue)->last_error;
				break;
#endif /* __QCQMI */

			default:
				printf("(EE) %s() queue %s not implemented\n", __func__, str_proto(mq->proto));
				break;
		}

		break;
	}

	return(res);
}

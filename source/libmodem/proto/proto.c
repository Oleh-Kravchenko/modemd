#include <stdio.h>

#include "proto.h"

#include "utils/sysfs.h"
#include "at/at_queue.h"
#include "qcqmi/qcqmi_queue.h"

/*------------------------------------------------------------------------*/

int modem_queues_init(modem_t* modem, const modem_db_device_t* mdd)
{
	char dev[0x100];
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

			case MODEM_PROTO_QCQMI:
				if(!modem_get_iface_dev(modem->port, "qcqmi", mdd->iface[i].num, dev, sizeof(dev)))
				{
					printf("(EE) Failed modem_get_iface_dev(%d)..\n", mdd->iface[i].num);

					goto err;
				}

				if(!(queue = qcqmi_queue_open(dev)))
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
	int i;

	for(i = 0; i < ARRAY_SIZE(modem->queues); ++ i)
	{
		switch(modem->queues[i].proto)
		{
			case MODEM_PROTO_AT:
				at_queue_destroy(modem->queues[i].queue);
				break;

			case MODEM_PROTO_QCQMI:
				qcqmi_queue_destroy(modem->queues[i].queue);
				break;

			default:
				printf("(WW) %s() Not implemented\n", __func__);
				break;
		}
	}
}

/*------------------------------------------------------------------------*/

void modem_queues_suspend(modem_t* modem)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(modem->queues); ++ i)
	{
		switch(modem->queues[i].proto)
		{
			case MODEM_PROTO_AT:
				at_queue_suspend(modem->queues[i].queue);
				break;

			case MODEM_PROTO_QCQMI:
				qcqmi_queue_suspend(modem->queues[i].queue);
				break;

			default:
				printf("(WW) %s() Not implemented\n", __func__);
				break;
		}
	}
}

/*------------------------------------------------------------------------*/

void modem_queues_resume(modem_t* modem)
{
	const modem_db_device_t* mdd;
	char dev[0x100];
	int i;

	mdd = modem_db_get_info(NULL, modem->usb.id_vendor, modem->usb.id_product);

	for(i = 0; i < ARRAY_SIZE(modem->queues); ++ i)
	{
		switch(modem->queues[i].proto)
		{
			case MODEM_PROTO_AT:
				modem_get_iface_dev(modem->port, "ttyUSB", mdd->iface[i].num, dev, sizeof(dev));
				at_queue_resume(modem->queues[i].queue, dev);
				break;

			case MODEM_PROTO_QCQMI:
				modem_get_iface_dev(modem->port, "qcqmi", mdd->iface[i].num, dev, sizeof(dev));
				qcqmi_queue_resume(modem->queues[i].queue, dev);
				break;

			default:
				break;
		}
	}
}

/*------------------------------------------------------------------------*/

void* modem_queues_get(modem_t* modem, modem_proto_t proto)
{
	int i;

	for(i = 0; modem->queues[i].proto && i < ARRAY_SIZE(modem->queues); ++ i)
		if(proto == modem->queues[i].proto)
			return(modem->queues[i].queue);

	return(NULL);
}

/*------------------------------------------------------------------------*/

int modem_queues_add(modem_t* modem, modem_proto_t proto, void* queue)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(modem->queues); ++ i)
	{
		if(MODEM_PROTO_NONE == modem->queues[i].proto)
		{
			modem->queues[i].queue = queue;

			return(0);
		}
	}

	return(i);
}

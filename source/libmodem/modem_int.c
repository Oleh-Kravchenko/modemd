#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include "hw/hw_common.h"
#include "utils/file.h"
#include "utils/sysfs.h"
#include "at/at_common.h"
#include "at/at_queue.h"
#include "qcqmi/qcqmi_queue.h"

/*------------------------------------------------------------------------*/

typedef struct modem_list_s
{
	modem_t* modem;

	struct modem_list_s* next;
} modem_list_t;

/*------------------------------------------------------------------------*/

modem_list_t* modems = NULL;

/*------------------------------------------------------------------------*/

void modem_close(modem_t* modem);

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

int modem_init(const char* socket_path)
{
	modems = NULL;

	return(0);
}

/*------------------------------------------------------------------------*/

void modem_cleanup(void)
{
	modem_list_t* item;

	/* forced modem close */
	while(modems)
		modem_close(modems->modem);
}

/*------------------------------------------------------------------------*/

modem_t* modem_open_by_port(const char* port)
{
	const modem_db_device_t* mdd;
	modem_t* res = NULL;
	modem_list_t* item;
	void* thread_res;
	int i;

	/* check if modem is already opened */
	for(item = modems; item; item = item->next)
	{
		if(strcmp(port, item->modem->port) == 0)
		{
			/* modem is already opened, increase refs */
			++ item->modem->refs;
			res = item->modem;

			/* and return handle */
			return(res);
		}
	}

	/* allocating memory for result */
	res = malloc(sizeof(*res));
	memset(res, 0, sizeof(*res));
	strncpy(res->port, port, sizeof(res->port) - 1);
	res->port[sizeof(res->port) - 1] = 0;

	/* check device present */
	if(!usb_device_get_info(port, &res->usb))
	{
		printf("(DD) Port %s power on..\n", port);

		/* device missing, maybe port power down? */
		port_power(port, 1);

		/* waiting for device ready */
		for(i = 10; !usb_device_get_info(port, &res->usb) && i > 0; -- i)
		{
			printf("(DD) Wait for modem ready..\n");

			sleep(1);
		}

		if(i == 0)
		{
			printf("(EE) Device missing..\n");

			goto err;
		}
	}

	/* check driver ready */
	if(!(mdd = modem_db_get_info(res->usb.product, res->usb.id_vendor, res->usb.id_product)))
		goto err;

	/* creating queues */
	if(modem_queues_init(res, mdd))
		goto err;

	/* starting registration routine */
	if(mdd->thread_reg)
		pthread_create(&res->reg.thread, NULL, (pthread_func_t)mdd->thread_reg, res);
	else
		res->reg.thread = 0;

	/* adding modem in pull */
	if(!(item = malloc(sizeof(*item))))
		goto err_insert;

	item->modem = res;
	item->next = modems;
	modems = item;

	return(res);

err_insert:
	/* termination registration routine */
	if(res->reg.thread)
	{
		res->reg.terminate = 1;

		pthread_join(res->reg.thread, &thread_res);
	}

	/* termination scan routine */
	if(res->scan.thread)
		pthread_join(res->scan.thread, &thread_res);

	/* destroying queues */
	modem_queues_destroy(res);

err:
	/* cleanup resources */
	port_power(port, 0);

	free(res);

	return(NULL);
}

/*------------------------------------------------------------------------*/

void modem_close(modem_t* modem)
{
	modem_list_t* item, *prev;
	void* thread_res;

	/* decrements modem clients */
	if(modem->refs > 0)
	{
		-- modem->refs;

		return;
	}

	/* termination registration routine */
	if(modem->reg.thread)
	{
		modem->reg.terminate = 1;

		pthread_join(modem->reg.thread, &thread_res);
	}

	/* termination scan routine */
	if(modem->scan.thread)
		pthread_join(modem->scan.thread, &thread_res);

	/* destroing queues */
	modem_queues_destroy(modem);

	/* power off modem */
	port_power(modem->port, 0);

	free(modem);

	/* removing modem from list */
	for(prev = item = modems; item; prev = item, item = item->next)
	{
		if(modem == item->modem)
		{
			if(item == modems)
				modems = item->next;
			else
				prev->next = item->next;

			free(item);

			break;
		}
	}
}

/*------------------------------------------------------------------------*/

char* modem_get_imei(modem_t* modem, char* imei, int len)
{
	if(!*modem->reg.state.imei)
		return(NULL);

	strncpy(imei, modem->reg.state.imei, len - 1);
	imei[len - 1] = 0;

	return(imei);
}

/*------------------------------------------------------------------------*/

int modem_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq)
{
	if(!modem->reg.ready)
		return(-1);

	memcpy(sq, &modem->reg.state.sq, sizeof(*sq));

	return(0);
}

/*------------------------------------------------------------------------*/

time_t modem_get_network_time(modem_t* modem)
{
	at_queue_t* q = modem->priv;

	return(at_get_network_time(q->queue));
}

/*------------------------------------------------------------------------*/

char* modem_get_imsi(modem_t* modem, char* imsi, int len)
{
	if(!*modem->reg.state.imsi)
		return(NULL);

	strncpy(imsi, modem->reg.state.imsi, len - 1);
	imsi[len - 1] = 0;

	return(imsi);
}

/*------------------------------------------------------------------------*/

char* modem_get_operator_name(modem_t* modem, char *oper, int len)
{
	if(!modem->reg.ready)
		return(NULL);

	if(*modem->reg.state.oper)
		strncpy(oper, modem->reg.state.oper, len - 1);
	else if(*modem->reg.state.oper_number)
		strncpy(oper, modem->reg.state.oper_number, len - 1);
	else
		return(NULL);
	
	oper[len - 1] = 0;

	return(oper);
}

/*------------------------------------------------------------------------*/

modem_network_reg_t modem_network_registration(modem_t* modem)
{
	return(modem->reg.state.reg);
}

/*------------------------------------------------------------------------*/

char* modem_get_network_type(modem_t* modem, char *network, int len)
{
	if(!modem->reg.ready)
		return(NULL);

	strncpy(network, modem->reg.state.network_type, len - 1);
	network[len - 1] = 0;

	return(network);
}

/*------------------------------------------------------------------------*/

int modem_change_pin(modem_t* modem, const char* old_pin, const char* new_pin)
{
	at_queue_t* at_q = modem->priv;

	return(at_change_pin(at_q->queue, old_pin, new_pin));
}

/*------------------------------------------------------------------------*/

modem_fw_ver_t* modem_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info)
{
	if(!modem->reg.state.fw_info.release)
		return(NULL);

	return(memcpy(fw_info, &modem->reg.state.fw_info, sizeof(*fw_info)));
}

/*------------------------------------------------------------------------*/

usb_device_info_t* modem_get_info(modem_t* modem, usb_device_info_t *mi)
{
	return(usb_device_get_info(modem->port, mi));
}

/*------------------------------------------------------------------------*/

int modem_operator_scan(modem_t* modem, modem_oper_t** opers)
{
	at_queue_t* q = modem->priv;

	return(at_operator_scan(q->queue, opers));
}

/*------------------------------------------------------------------------*/

int modem_operator_scan_start(modem_t* modem, const char* file)
{
	at_operator_scan_t *at_priv;
	at_queue_t* at_q = modem->priv;
	int res = 0;

	if(modem->scan.thread)
		goto exit;

	if(!(at_priv = malloc(sizeof(*at_priv))))
		goto exit;

	/* filename */
	strncpy(at_priv->file, file, sizeof(at_priv->file) - 1);
	at_priv->file[sizeof(at_priv->file) - 1] = 0;
	at_priv->queue = at_q->queue;

	/* creating thread with a query AT+COPS=? */
	if((res = pthread_create(&modem->scan.thread, NULL, at_thread_operator_scan, at_priv)))
		free(at_priv);

exit:
	return(res);
}

/*------------------------------------------------------------------------*/

int modem_operator_scan_is_running(modem_t* modem)
{
	void *thread_res;
	int res = -1;

	if(modem->scan.thread)
		res = pthread_kill(modem->scan.thread, 0);

	if(res == ESRCH)
	{
		pthread_join(modem->scan.thread, &thread_res);
		modem->scan.thread = 0;
		res = 0;
	}
	else if(res == 0)
		res = 1;

	return(res);
}

/*------------------------------------------------------------------------*/

int modem_get_cell_id(modem_t* modem)
{
	at_queue_t* q = modem->priv;

	return(at_get_cell_id(q->queue));
}

/*------------------------------------------------------------------------*/

char* modem_at_command(modem_t* modem, const char* query)
{
	at_queue_t* at_q = modem->priv;
	at_query_t *q;
	char *cmd;

	if(!at_q)
		return(NULL);

	/* formating query */
	if(!(cmd = malloc(strlen(query) + 1 + 2))) /* +2 for "\r\n" */
		return(NULL);

	strcpy(cmd, query);
	strcat(cmd, "\r\n");

	q = at_query_create(cmd, "OK\r\n");
	q->timeout = 10;

	at_query_exec(at_q->queue, q);

	free(cmd);
	cmd = NULL;

	if(q->result)
		cmd = strdup(q->result);

	at_query_free(q);

	return(cmd);
}

/*------------------------------------------------------------------------*/

int modem_get_last_error(modem_t* modem)
{
	return(modem->reg.last_error);
}

/*------------------------------------------------------------------------*/

void modem_conf_reload(modem_t* modem)
{
	const modem_db_device_t* mdd;
	usb_device_info_t mi;
	void* thread_res;

	if(!modem->reg.thread)
		return;

	modem->reg.terminate = 1;
	pthread_join(modem->reg.thread, &thread_res);

	usb_device_get_info(modem->port, &mi);
	mdd = modem_db_get_info(NULL, mi.id_vendor, mi.id_product);

	modem->reg.terminate = 0;

	/* starting registration routine */
	if(mdd && mdd->thread_reg)
		pthread_create(&modem->reg.thread, NULL, (pthread_func_t)mdd->thread_reg, modem);
	else
		modem->reg.thread = 0;
}

/*------------------------------------------------------------------------*/

void modem_reset(modem_t* modem)
{
	void *thread_res;

	/* termination scan routine */
	if(modem->scan.thread)
		pthread_join(modem->scan.thread, &thread_res);

	/* suspend queues */
	modem_queues_suspend(modem);

	/* reseting modem */
	port_reset(modem->port);

	sleep(10);

	/* resume queues */
	modem_queues_resume(modem);
}

#ifndef __QCQMI_QUEUE_H
#define __QCQMI_QUEUE_H

#include <time.h>
#include <pthread.h>

#include "modem/types.h"

/*------------------------------------------------------------------------*/

typedef struct serving_system_info_s
{
	time_t next_update;
	
	unsigned long reg_state;

	unsigned long cs_domain;

	unsigned long ps_domain;

	unsigned long ran;

	unsigned long roaming;

	uint8_t network_type_size;

	uint8_t network_type[0x32];

	uint16_t mcc;

	uint16_t mnc;

	char oper_name[0x32];
} serving_system_info_t;

/*------------------------------------------------------------------------*/

typedef struct
{
	int terminate;

	unsigned long last_error;

	pthread_mutex_t mutex;

	serving_system_info_t state;
} qcqmi_queue_t;

/*------------------------------------------------------------------------*/

qcqmi_queue_t* qcqmi_queue_open(const char* dev);

void qcqmi_queue_destroy(qcqmi_queue_t* queue);

void qcqmi_queue_suspend(qcqmi_queue_t* queue);

void qcqmi_queue_resume(qcqmi_queue_t* queue, const char* dev);

#endif /* __QCQMI_QUEUE_H */

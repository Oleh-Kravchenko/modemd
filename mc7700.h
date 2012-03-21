#ifndef __MC7700_H
#define __MC7700_H

#include "queue.h"

typedef struct {
	int fd;

	pthread_cond_t cond;

	queue_t *q;

	int terminate;
} thread_queue_t;

extern thread_queue_t thread_priv;

int mc7700_open(const char *port);

void mc7700_destroy(void);

void* mc7700_thread_write(void* prm);

void* mc7700_thread_read(void* prm);

#endif /* __MC7700_H */

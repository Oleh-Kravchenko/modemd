#ifndef __AT_QUEUE_H
#define __AT_QUEUE_H

#include <regex.h>

#include "at/at_query.h"
#include "modem/types.h"
#include "../libmodem/queue.h"
 
/*------------------------------------------------------------------------*/

typedef struct
{
    int fd;

    pthread_cond_t processed;

    pthread_mutex_t mutex;

    queue_t *q;

    at_query_t* query;

    int terminate;

    pthread_t thread_write;

    pthread_t thread_read;

    int32_t last_error;
} at_queue_t;

/*------------------------------------------------------------------------*/

at_queue_t* at_queue_open(const char *tty);

void at_queue_destroy(at_queue_t* priv);

#endif /* __AT_QUEUE_H */

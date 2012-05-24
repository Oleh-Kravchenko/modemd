#ifndef __AT_QUERY_H
#define __AT_QUERY_H

#include <regex.h>

#include "modem/types.h"
#include "../queue.h"

/*------------------------------------------------------------------------*/

typedef struct at_query_s
{
    char* cmd;

    char* re_res;

    int n_subs;

    regmatch_t *re_subs;

    /** timeout for command in seconds */
    int timeout;

    char* result;

    pthread_cond_t cond;

    pthread_mutex_t cond_m;

    /** must be -1 if no errors after execute */
    int error;
} at_query_t;

/*------------------------------------------------------------------------*/

at_query_t* at_query_create(const char* query, const char* answer_reg);

/*------------------------------------------------------------------------*/

int at_query_exec(queue_t* q, at_query_t* query);

/*------------------------------------------------------------------------*/

int at_query_is_error(at_query_t* query);

/*------------------------------------------------------------------------*/

void at_query_free(at_query_t* query);

#endif /* __AT_QUERY_H */
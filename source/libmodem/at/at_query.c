#include "../queue.h"
#include "at_query.h"

/*------------------------------------------------------------------------*/

at_query_t* at_query_create(const char* q, const char* reply_re)
{
    at_query_t* res;

    if(!(res = malloc(sizeof(*res))))
        goto err;

    if(!(res->cmd = malloc(strlen(q) + 1)))
        goto err_q;

    memcpy(res->cmd, q, strlen(q) + 1);
    res->cmd[strlen(q)] = 0;

    if(!(res->re_res = malloc(strlen(reply_re) + 1)))
        goto err_a;

    memcpy(res->re_res, reply_re, strlen(reply_re) + 1);
    res->re_res[strlen(reply_re)] = 0;

    res->result = NULL;
    res->re_subs = NULL;
    res->n_subs = 0;
    res->timeout = 2; /* default timeout for command */
    res->error = -1;  /* -1 is no error */

    pthread_cond_init(&res->cond, NULL);
    pthread_mutex_init(&res->cond_m, NULL);

    goto exit;

err_a:
    free(res->cmd);

err_q:
    free(res);
    res = NULL;

err:
exit:
    return(res);
}

/*------------------------------------------------------------------------*/

int at_query_exec(queue_t* queue, at_query_t* query)
{
    int res;

    if((res = queue_add(queue, &query, sizeof(at_query_t**))))
        goto err;

    /* wait for processing */
    pthread_mutex_lock(&query->cond_m);
    pthread_cond_wait(&query->cond, &query->cond_m);
    pthread_mutex_unlock(&query->cond_m);

err:
    return(res);
}

/*------------------------------------------------------------------------*/

int at_query_is_error(at_query_t* query)
{
	return(query->error != -1);
}

/*------------------------------------------------------------------------*/

void at_query_free(at_query_t* q)
{
    if(!q)
        return;

    free(q->result);
    free(q->re_res);
    free(q->cmd);
    pthread_cond_destroy(&q->cond);
    pthread_mutex_destroy(&q->cond_m);
    free(q->re_subs);
    free(q);
}

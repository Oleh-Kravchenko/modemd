#include "queue.h"
#include "at/at_query.h"

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
	res->pmatch = NULL;
	res->nmatch = 0;
	res->timeout = 2; /* default timeout for command */
	res->error = -1;  /* -1 is no error */

	res->event = event_create();

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
	{
		query->error = 0;

		goto err;
	}

	/* wait for processing */
	event_wait(query->event);

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

	event_destroy(q->event);

	free(q->result);
	free(q->re_res);
	free(q->cmd);
	free(q->pmatch);
	free(q);
}

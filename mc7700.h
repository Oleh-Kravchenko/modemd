#ifndef __MC7700_H
#define __MC7700_H

#include <regex.h>

#include "modem/types.h"
#include "queue.h"

/*------------------------------------------------------------------------*/

typedef struct mc7700_query_s
{
    char* query;

    char* answer_reg;

    int n_subs;

    regmatch_t *re_subs;

    /** timeout for command in seconds */
    int timeout;

    char* answer;

    pthread_cond_t cond;

    pthread_mutex_t cond_m;

    /* must be -1 if no errors after execute */
    int error;
} mc7700_query_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    int from_file;

    char pin[20];

    char puk[20];

    modem_data_profile_t data;

    int roaming;

    int operator_number;

    int access_technology;

    int frequency_band;
} modem_conf_t;
 
/*------------------------------------------------------------------------*/

typedef struct
{
    int fd;

    char port[0x100];

    char tty[0x100];

    pthread_cond_t processed;

    pthread_mutex_t mutex;

    queue_t *q;

    int terminate;

    int terminate_reg;

    int locked;

    int mc7700_clients;

    pthread_t thread_write;

    pthread_t thread_read;

    pthread_t thread_reg;

    pthread_t thread_scan;

    mc7700_query_t* query;

    modem_conf_t conf;

    int32_t last_error;
    
    /* cached values, update per 10 seconds */

    modem_network_reg_t reg;
    
    modem_signal_quality_t sq;

    char oper[0x100];
    
    char network_type[0x32];
    
    /* values valid per session */

    modem_fw_version_t fw_info;
    
    char imsi[0x32];

    char imei[0x32];

} thread_queue_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    char file[0x100];

    thread_queue_t *priv;
} at_operator_scan_t;

/*------------------------------------------------------------------------*/

extern thread_queue_t mc7700_thread_priv;

/*------------------------------------------------------------------------*/

mc7700_query_t* mc7700_query_create(const char* query, const char* answer_reg);

int mc7700_query_execute(queue_t* q, mc7700_query_t* query);

void mc7700_query_destroy(mc7700_query_t* query);

/*------------------------------------------------------------------------*/

int mc7700_open(const char *port, const char *tty);

void mc7700_destroy(void);

/*------------------------------------------------------------------------*/

void* mc7700_thread_write(void* prm);

void* mc7700_thread_read(void* prm);

void* mc7700_thread_reg(void* prm);

/*------------------------------------------------------------------------*/

void mc7700_read_config(const char* port, modem_conf_t* conf);

#endif /* __MC7700_H */

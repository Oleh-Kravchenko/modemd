#ifndef __AT_COMMON_H

#include <modem/types.h>

#include "queue.h"

/*------------------------------------------------------------------------*/

typedef struct
{
	char file[0x100];

	queue_t* queue;
} at_operator_scan_t;

/*------------------------------------------------------------------------*/

modem_cpin_state_t at_cpin_state(queue_t* queue);

/*------------------------------------------------------------------------*/

int at_cpin_pin(queue_t* queue, const char* pin);

/*------------------------------------------------------------------------*/

int at_cpin_puk(queue_t* queue, const char* puk, const char* pin);

/*------------------------------------------------------------------------*/

/**
 * @brief execute AT command on AT queue
 * @param queue AT queue
 * @param cmd AT command
 * @return zero if no errors
 */
int at_raw_ok(queue_t* queue, const char* cmd);

/*------------------------------------------------------------------------*/

char* at_get_imsi(queue_t* queue, char* imsi, size_t len);

/*------------------------------------------------------------------------*/

char* at_get_imei(queue_t* queue, char* imsi, size_t len);

/*------------------------------------------------------------------------*/

int at_operator_scan(queue_t* queue, modem_oper_t** opers);

/**
 * @brief background operator scanner
 * @remark this function only for openrg
 */
void* at_thread_operator_scan(void* prm);

/*------------------------------------------------------------------------*/

modem_network_reg_t at_network_registration(queue_t* queue);

/*------------------------------------------------------------------------*/

modem_cops_mode_t at_cops_mode(queue_t* queue);

/*------------------------------------------------------------------------*/

int at_get_signal_quality(queue_t *queue, modem_signal_quality_t* sq);

/*------------------------------------------------------------------------*/

modem_fw_ver_t* at_get_fw_version(queue_t *queue, modem_fw_ver_t* fw_info);

/*------------------------------------------------------------------------*/

char* at_get_network_type(queue_t *queue, char *network, int len);

/*------------------------------------------------------------------------*/

char* at_get_operator_name(queue_t *queue, char *oper, int len);

/*------------------------------------------------------------------------*/

char* at_get_operator_number(queue_t *queue, char *oper_number, int len);

/*------------------------------------------------------------------------*/

int at_get_cell_id(queue_t *queue);

/*------------------------------------------------------------------------*/

time_t at_get_network_time(queue_t *queue);

/*------------------------------------------------------------------------*/

char* at_get_ccid(queue_t *queue, char* ccid, size_t len);

/*------------------------------------------------------------------------*/

int at_change_pin(queue_t *queue, const char* old_pin, const char* new_pin);

#endif /* __AT_COMMON_H */

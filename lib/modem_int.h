#ifndef __MODEM_INTERNAL_H

#include <dirent.h>

#include "modem/types.h"

#include "../queue.h"

/*------------------------------------------------------------------------*/

modem_info_t* usb_device_get_info(const char* port);

modem_info_t* modem_find_first(DIR **dir);

modem_info_t* modem_find_next(DIR **dir);

/*------------------------------------------------------------------------*/

modem_cpin_state_t at_cpin_state(queue_t* queue);

int at_cpin_pin(queue_t* queue, const char* pin);

int at_cpin_puk(queue_t* queue, const char* puk, const char* pin);

int at_raw_ok(queue_t* queue, const char* cmd);

char* at_get_imsi(queue_t* queue, char* imsi, int len);

char* at_get_imei(queue_t* queue, char* imsi, int len);

int at_operator_scan(queue_t* queue, modem_oper_t** opers);

/**
 * @brief background operator scanner
 * @remark this function only for openrg
 */
void* at_thread_operator_scan(void* prm);

modem_network_reg_t at_network_registration(queue_t* queue);

modem_cops_mode_t at_cops_mode(queue_t* queue);

int at_get_signal_quality(queue_t *queue, modem_signal_quality_t* sq);

modem_fw_version_t* at_get_fw_version(queue_t *queue, modem_fw_version_t* fw_info);

char* at_get_network_type(queue_t *queue, char *network, int len);

char* at_get_operator_name(queue_t *queue, char *oper, int len);

#endif /* __MODEM_INTERNAL_H */

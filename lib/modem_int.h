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

int at_operator_scan(queue_t* queue, modem_oper_t** opers);

/**
 * @brief background operator scanner
 * @remark this function only for openrg
 */
void* at_thread_operator_scan(void* prm);

#endif /* __MODEM_INTERNAL_H */

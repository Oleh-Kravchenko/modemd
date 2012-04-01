#ifndef __MODEM_H
#define __MODEM_H

#include <time.h>

#include "modem/types.h"

/***************************************************************************
 * Initialization                                                          *
 **************************************************************************/

/**
 * @brief initialize library
 * @param socket_path path to the local socket
 * @return zero, if successful
 *
 * Must be called only once per program.
 */
int modem_init(const char* socket_path);

/** deinitialize library */
void modem_cleanup(void);

/**
 * @brief return first modem
 * @return pointer to modem_info_t, zero if no modem detected
 *
 * The result must be function free()
 */
modem_info_t* modem_find_first(void);

/**
 * @brief return next modem
 * @return pointer to modem_info_t, zero if no modem left
 *
 * The result must be function free()
 */
modem_info_t* modem_find_next(void);

/***************************************************************************
 * Modem                                                                   *
 **************************************************************************/

/**
 * @brief open modem by port and return handle
 * @param port port name
 * @return modem handle 
 *
 * Modem handle must be close by modem_close()
 */
modem_t* modem_open_by_port(const char* port);

/**
 * @brief close modem handle
 * @param modem handle
 */
void modem_close(modem_t* modem);

/**
 * @brief return IMEI of modem
 * @param modem handle
 * @param imei buffer
 * @param len length of imei buffer
 * @return if successful, return imei buffer. otherwise 0
 */
char* modem_get_imei(modem_t* modem, char* imei, int len);

int modem_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq);

time_t modem_get_network_time(modem_t* modem);

char* modem_get_imsi(modem_t* modem, char* imsi, int len);

/**
 * @brief return operator name
 * @param modem modem handle
 * @param oper pointer of buffer for operator name
 * @param len length of buffer
 * @return if successful pointer to oper, otherwise NULL
 */
char* modem_get_operator_name(modem_t* modem, char *oper, int len);

modem_network_reg_t modem_network_registration(modem_t* modem);

char* modem_get_network_type(modem_t* modem, char *network, int len);

#if 0

/***************************************************************************
 * EVENT                                                                   *
 **************************************************************************/

modem_event_t* modem_wait_event(modem_t* modem, modem_event_t* event, int timeout);

int modem_register_event_handler_callback(modem_t* modem, modem_event_handler_t* event_handler);

void modem_unregister_event_handler_callback(modem_t* modem, modem_event_handler_t* event_handler);

/***************************************************************************
 * DATA SESSION                                                            *
 **************************************************************************/

int modem_max_number_of_data_profiles(modem_t* modem);

int modem_data_profile_setup(modem_t* modem, int slot, modem_data_profile_t* profile);

modem_data_profile_t* modem_data_profile_read(modem_t* modem, int slot, modem_data_profile_t* profile);

int modem_data_profile_clear(modem_t* modem, int slot);

int modem_data_profile_connect(modem_t* modem, int slot);

void modem_data_profile_disconnect(modem_t* modem, int slot);

int modem_data_profile_is_active(modem_t* modem, int slot);

#endif /* 0 */

#endif /* __MODEM_H */

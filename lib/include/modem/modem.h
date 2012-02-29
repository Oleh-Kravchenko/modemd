#ifndef __MODEM_H
#define __MODEM_H

#include "modem/types.h"

/***************************************************************************
 * Initialization                                                          *
 **************************************************************************/

int modem_init(const char* socket_path);

void modem_cleanup(void);

modem_info_t* modem_find_first(void);

modem_info_t* modem_find_next(void);

int modem_timeout_setup(int timeout);

/***************************************************************************
 * Modem                                                                   *
 **************************************************************************/
#if 0
modem_t* modem_open_by_iface(const char* iface);

modem_t* modem_open_by_port(const char* port);

int modem_reset_hw(modem_t* modem);

int modem_reset_sw(modem_t* modem);

void modem_close(modem_t* modem);

int modem_set_pin(modem_t* modem, const char* pin);

int modem_set_pin_puk(modem_t* modem, const char* pin, const char* puk);

int modem_change_pin_code(modem_t* modem, const char* pin, const char* new_pin);

int modem_sim_is_ready(modem_t* modem);

char* modem_get_imsi(modem_t* modem, char* imsi, int len);

char* modem_get_imei(modem_t* modem, char* imei, int len);

char* modem_get_hni(modem_t* modem, char* hni, int len);

char* modem_get_operator_hni(modem_t* modem, char* hni, int len);

char* modem_get_operator_name(modem_t* modem, char* operator_name, int len);

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

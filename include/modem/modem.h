#ifndef __MODEM_H
#define __MODEM_H

#include <time.h>

#include <modem/types.h>
#include <modem/modem_errno.h>

/***************************************************************************
 * function for initialization                                             *
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
 */
modem_find_t* modem_find_first(usb_device_info_t* mi);

/**
 * @brief return next modem
 * @return pointer to modem_info_t, zero if no modem left
 */
modem_find_t* modem_find_next(modem_find_t* find, usb_device_info_t* mi);

/**
 * @brief terminate find proccess
 * @param mi find handle
 */
void modem_find_close(modem_find_t* find);

/***************************************************************************
 * function for modem handling                                             *
 **************************************************************************/

/**
 * @brief open modem by port and return handle
 * @param port port name
 * @return modem handle 
 *
 * Modem handle must be closed by modem_close()
 */
modem_t* modem_open_by_port(const char* port);

/**
 * @brief close modem handle
 * @param modem handle
 */
void modem_close(modem_t* modem);

void modem_conf_reload(modem_t* modem);

usb_device_info_t* modem_get_info(modem_t* modem, usb_device_info_t *mi);

/**
 * @brief return last error on modem
 * @param modem handle
 * @return if no errors result is -1
 */
int modem_get_last_error(modem_t* modem);

/**
 * @brief return last registration error on modem
 * @param modem handle
 * @return if no errors result is -1
 */
int modem_get_last_reg_error(modem_t* modem);

/**
 * @brief execute AT command on modem
 * @param modem modem handle
 * @param query AT query for modem
 * @return result for command
 *
 * After usage must be called free() for result
 */
char* modem_at_command(modem_t* modem, const char* query);

/***************************************************************************
 * functions implemented per modem                                         *
 **************************************************************************/

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

/**
 * @brief change pin code
 * @param modem modem handle
 * @param old_pin old pin code
 * @param new_pin new pin code
 * @return 0 if successful
 */
int modem_change_pin(modem_t* modem, const char* old_pin, const char* new_pin);

modem_fw_ver_t* modem_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info);

/**
 * @brief perform operator scan
 * @param modem modem handle
 * @param opers pointer to array of items
 * @return numbers of items in opers
 *
 * After usage must be called free(opers)
 */
int modem_operator_scan(modem_t* modem, modem_oper_t** opers);

/**
 * @brief return cell id
 * @param modem modem handle
 * @return cell id, if failed 0
 */
int modem_get_cell_id(modem_t* modem);

int modem_set_default_profile(modem_data_profile_t* profile);

/**
 * @brief execute USSD command on modem
 * @param modem modem handle
 * @param query USSD query for modem
 * @return result for command
 *
 * Example: *111#
 * After usage must be called free() for result
 */
char* modem_ussd_cmd(modem_t* modem, const char* query);

/***************************************************************************
 * functions for openrg                                                    *
 **************************************************************************/

/**
 * @brief perform operator scan and save result in file
 * @param file save operator list in this file
 * @return 0 if scan is started
 * @remark this function only for openrg
 */
int modem_operator_scan_start(modem_t* modem, const char* file);

/**
 * @brief report about background operator scan
 * @return -1 error, 0 not running, 1 scanning is running
 * @remark this function only for openrg
 */
int modem_operator_scan_is_running(modem_t* modem);

/***************************************************************************
 * functions for wwan                                                      *
 **************************************************************************/

int modem_set_wwan_profile(modem_t* modem, modem_data_profile_t* profile);

int modem_start_wwan(modem_t* modem);

int modem_stop_wwan(modem_t* modem);

modem_state_wwan_t modem_state_wwan(modem_t* modem);

#endif /* __MODEM_H */

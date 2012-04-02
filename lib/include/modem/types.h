#ifndef __MODEM_TYPES_H
#define __MODEM_TYPES_H

#include <stdint.h>

/*------------------------------------------------------------------------*/

typedef void* modem_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    /** port number */
    char port[8];

    /** vendor name */
    uint16_t id_vendor;

    /** product name */
    uint16_t id_product;

    /** manufacturer name */
    char manufacturer[256];

    /** product name */
    char product[256];
} __attribute__((__packed__)) modem_info_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    /* signal quality in dBm */
    int16_t dbm;

    /* signal level 1-5 */
    uint8_t level;
} __attribute__((__packed__)) modem_signal_quality_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    PPP_NONE = 0,
    PPP_PAP,
    PPP_CHAP
} __attribute__((__packed__)) ppp_auth_protocol_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    /** APN */
    char apn[101];

    /** username */
    char username[33];

    /** password */
    char password[33];

    /** type of authrization (None, PAP, CHAP) */
    ppp_auth_protocol_t auth;
} __attribute__((__packed__)) modem_data_profile_t;

/*------------------------------------------------------------------------*/

typedef enum
{
	MODEM_NETWORK_REG_FAILED    = 0,
	MODEM_NETWORK_REG_HOME,
	MODEM_NETWORK_REG_SEARCHING,
	MODEM_NETWORK_REG_DENIED,
	MODEM_NETWORK_REG_UNKNOWN,
	MODEM_NETWORK_REG_ROAMING
} __attribute__((__packed__)) modem_network_reg_t;

/*------------------------------------------------------------------------*/

typedef struct
{
	char old_pin[16];

	char new_pin[16];
} __attribute__((__packed__)) modem_change_pin_t;

/*------------------------------------------------------------------------*/

#if 0

typedef void* modem_event_t;

typedef void* modem_event_handler_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    MODEM_EVENT_NOOP = 0,
    MODEM_EVENT_SIM_NOT_READY,
    MODEM_EVENT_PIN_NOT_READY,
    MODEM_EVENT_PUK_NOT_READY,
    MODEM_EVENT_RESET_SW,
    MODEM_EVENT_RESET_HW,
    MODEM_EVENT_REGISTRATION,
    MODEM_EVENT_SIGNAL_QUALITY,
    MODEM_EVENT_DATA_SESSION,
    MODEM_EVENT_SERVER_RESTART,
    MODEM_EVENT_SERVER_SHUTDOWN
} modem_event_type_t;

/*------------------------------------------------------------------------*/

#endif /* 0 */

#endif /* __MODEM_TYPES_H */

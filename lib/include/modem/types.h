#ifndef __MODEM_TYPES_H
#define __MODEM_TYPES_H

#include <stdint.h>

typedef struct
{
    /// port number
    char port[8];

    /// vendor name
    uint16_t id_vendor;

    /// product name
    uint16_t id_product;

    /// manufacturer name
    char manufacturer[256];

    /// product name
    char product[256];
} __attribute__((__packed__)) modem_info_t;


/*------------------------------------------------------------------------*/

typedef void* modem_t;

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

typedef enum
{
    PPP_NONE = 0,
    PPP_PAP,
    PPP_CHAP
} ppp_auth_protocol_t;

/*------------------------------------------------------------------------*/

typedef struct
{
    /// APN
    char apn[101];

    /// username
    char username[33];

    /// password
    char password[33];

    /// type of authrization (None, PAP, CHAP)
    ppp_auth_protocol_t auth;
} modem_data_profile_t;

#endif /* 0 */

#if 0
typedef enum
{
   eNOT_REGISTERED_NOT_SEARCHING = 0,
   eREGISTERED_HOME,
   eNOT_REGISTERED_SEARCHING,
   eNOT_REGISTERED_SEARCHING_BASE_ST,  //Artem for CDMA
   eREGISTRATION_DENIED,
   eREGISTRATION_UNKNOWN,
   eREGISTERED_ROAMING,
//   eLIMITED_SERVICE                    //@Anatoly - ticket #2111
} TCellEngineRegStatus_e;

typedef enum
{
   eERR_NO_ERROR            = 0,
   eERR_CONFIG_LOAD,
   eERR_NO_RESPONSE,
   eERR_NO_SIM,
   eERR_WIND3_MISSING,
   eERR_WIND4_7_MISSING,
   eERR_NETWORK_LOST,
   eERR_FREQ_BAND_SELECTION,
   eERR_PIN_REQUIRED,
   eERR_PUK_REQUIRED,
   eERR_WRONG_PIN_PUK,
   eERR_PIN2_REQUIRED,
   eERR_PUK2_REQUIRED,
   eERR_WRONG_PIN2_PUK2,
   eERR_CCID_LOCK,
   eERR_MCC_LOCK,
   eERR_MNC_LOCK,
   eERR_MSIN_LOCK,
   eERR_OPERATOR_SELECTION,
   eERR_REGISTRATION_DENIED,
   eERR_ROAMING_RESTRICTED,
   eERR_ERROR_515,
   eERR_ERROR_519,
   eERR_AUTOMATIC_RSSI,
   eERR_SIM_BLOCKED,
   eERR_SIM_BUSY,
   eERR_SIM_INVALID,
   eERR_REGISTRATION_IN_PROGRESS,
   eERR_UNKNOWN_ERROR,
   eERR_MAX_NUMBER
} TCellRegistrationFailure_e;
#endif //0

#endif /* __MODEM_TYPES_H */

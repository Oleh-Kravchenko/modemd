#ifndef __MODEM_H
#define __MODEM_H

#include <stdint.h>
#include <sys/types.h>
#include <dirent.h>

/*------------------------------------------------------------------------*/

typedef void* modem_event_t;

typedef void* modem_t;

typedef struct
{
    /// номер порта
    uint8_t port[8];

    /// индификатор производителя
    uint16_t id_vendor;

    /// индификатор продукта
    uint16_t id_product;

    /// имя производителя
    uint8_t manufacturer[256];

    /// имя продукта
    uint8_t product[256];

    DIR *sysfs_dir;
} modem_info_t;

typedef void* modem_event_handler_t;

/*------------------------------------------------------------------------*/

typedef enum
{
    MODEM_EVENT_NOOP = 0,        ///< пустое событие, для тестирования
    MODEM_EVENT_SIM_NOT_READY,   ///< СИМ-карта не готова
    MODEM_EVENT_PIN_NOT_READY,   ///< требуется ПИН-код
    MODEM_EVENT_PUK_NOT_READY,   ///< требуется ПАК-код
    MODEM_EVENT_RESET_SW,        ///< запрошен программный сброс модема
    MODEM_EVENT_RESET_HW,        ///< запрошен аппаратный сброс модема
    MODEM_EVENT_REGISTRATION,    ///< состояние регистрации изменилось
    MODEM_EVENT_SIGNAL_QUALITY,  ///< уровень сигнала изменился
    MODEM_EVENT_DATA_SESSION,    ///< состояние интернет соединения изменилось
    MODEM_EVENT_SERVER_RESTART,  ///< cellulard будет перезапущен
    MODEM_EVENT_SERVER_SHUTDOWN  ///< cellulard завершает работу
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

    /// имя пользователя
    char username[33];

    /// пароль
    char password[33];
    
    /// тип авторизации
    ppp_auth_protocol_t auth;
} modem_data_profile_t;

/*------------------------------------------------------------------------*/

/***************************************************************************
 * Initialization                                                          *
 **************************************************************************/
/**
 * @brief инициализация
 * @param socket_path путь к сокету сервера
 * @return 0, если успешно
 *
 * Функция должна быть вызвана перед использованием любых других функций.
 */
int modem_init(const char* socket_path);

/// деинициализация
void modem_cleanup(void);

/**
 * @brief возвращает информацию о первом модеме
 * @return указатель на описание модема, NULL если модем отсутствует
 */
modem_info_t* modem_find_first(void);

/**
 * @brief Возвращает информацию о следующем модеме
 * @param modem указатель на описание модема
 * @return NULL, если перечислены все модемы
 */
modem_info_t* modem_find_next(modem_info_t* modem);

/**
 * @brief устанавливает таймаут на вызов функций модема
 * @param timeout таймаут
 * @return 0, если успешно
 *
 * timeout очереди
 */
int modem_timeout_setup(int timeout);

/***************************************************************************
 * Modem                                                                   *
 **************************************************************************/

/**
 * @brief идентифицирует модем по сетевому интерфейсу и возвращает его хендл
 * @param iface имя сетевого интерфейса eth0, usb0, etc.
 * @return хендл модема 
 */
modem_t* modem_open_by_iface(const char* iface);

/**
 * @brief идентифицирует модем по номеру порта и возвращает его хендл
 * @param port имя порта в формате, BUS-DEV (номер шины, номер ус-ва)
 * @return хендл модема 
 */
modem_t* modem_open_by_port(const char* port);

/**
 * @brief аппаратный сброс модема
 * @param modem хендл модема
 * @return 0, в случае успеха
 * 
 * Сброс модема выполняется через Lattice.
 */
int modem_reset_hw(modem_t* modem);

/**
 * @brief программный сброс модема
 * @param modem хендл модема
 * @return 0, в случае успеха
 * 
 * Сброс модема выполняется путем посылки команды модему (например, AT!RESET)
 */
int modem_reset_sw(modem_t* modem);

/**
 * @brief закрывает хендл модема
 * @param modem хендл модема
 */
void modem_close(modem_t* modem);

/**
 * @brief посылает ПИН-код
 * @param modem хендл модема
 * @param pin ПИН-код
 * @return 0 если успешно
 */
int modem_set_pin(modem_t* modem, const char* pin);

/**
 * @brief посылает ПАК-код
 * @param modem хендл модема
 * @param pin ПИН-код
 * @param puk ПАК-код
 * @return 0 если успешно
 */
int modem_set_pin_puk(modem_t* modem, const char* pin, const char* puk);

/**
 * @brief меняет ПИН-код установленный на СИМ-карте
 * @param modem хендл модема
 * @param pin ПИН-код
 * @param new_pin новый ПИН-код
 * @return 0 если успешно
 */
int modem_change_pin_code(modem_t* modem, const char* pin, const char* new_pin);

/**
 * @brief проверяет состояние СИМ-карты
 * @param modem хендл модема
 * @return 0 если СИМ-карта не готова
 */
int modem_sim_is_ready(modem_t* modem);

/**
 * @brief получает IMSI СИМ-карты
 * @param modem хендл модема
 * @param imsi указатель на буфер
 * @param len размер буфера
 * @return указатель на буфер imsi, в случае ошибки NULL
 */
char* modem_get_imsi(modem_t* modem, char* imsi, int len);

/**
 * @brief получает IMEI модема
 * @param modem хендл модема
 * @param imei указатель на буфер
 * @param len размер буфера
 * @return указатель на буфер imei, в случае ошибки NULL
 */
char* modem_get_imei(modem_t* modem, char* imei, int len);

/**
 * @brief получает HNI СИМ-карты
 * @param modem хендл модема
 * @param hni указатель на буфер
 * @param len размер буфера
 * @return указатель на буфер hni, в случае ошибки NULL
 */
char* modem_get_hni(modem_t* modem, char* hni, int len);

/**
 * @brief получает HNI оператора
 * @param modem хендл модема
 * @param hni указатель на буфер
 * @param len размер буфера
 * @return указатель на буфер hni, в случае ошибки NULL
 */
char* modem_get_operator_hni(modem_t* modem, char* hni, int len);

/**
 * @brief получает имя оператора
 * @param modem хендл модема
 * @param operator_name указатель на буфер
 * @param len размер буфера
 * @return указатель на буфер operator_name, в случае ошибки NULL
 */
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
/**
 * @brief возращает количество слотов для профилей интернет соединения
 * @param modem хендл модема
 * @return количество слотов для профилей в модеме
 */
int modem_max_number_of_data_profiles(modem_t* modem);

/**
 * @brief устанавливает профиль интернет соединения
 * @param modem хендл модема
 * @param slot номер слота
 * @param profile указатель на структуру с данными профиля
 * @return 0, в случае успеха
 */
int modem_data_profile_setup(modem_t* modem, int slot, modem_data_profile_t* profile);

/**
 * @brief читает данные профиля из модема
 * @param modem хендл модема
 * @param slot номер профиля
 * @param profile указатель на структуру профиля
 * @return указатель на профиль, или NULL в случае ошибки
 */
modem_data_profile_t* modem_data_profile_read(modem_t* modem, int slot, modem_data_profile_t* profile);

/**
 * @brief очищает профиль
 * @param modem хендл модема
 * @param slot номер профиля
 * @return 0, если успешно
 */
int modem_data_profile_clear(modem_t* modem, int slot);

/**
 * @brief подключает профиль
 * @param modem хендл модема
 * @param slot номер профиля
 * @return 0, если команда на соединение выполнена успешно
 */
int modem_data_profile_connect(modem_t* modem, int slot);

/**
 * @brief отключает соединение
 * @param modem хендл модема
 * @param slot номер профиля
 */
void modem_data_profile_disconnect(modem_t* modem, int slot);

/**
 * @brief возращает состояние профиля соединения
 * @param modem хендл модема
 * @param slot номер профиля
 * @return 0, если соединение на профиле номер @slot не активно
 */
int modem_data_profile_is_active(modem_t* modem, int slot);

#endif /* __MODEM_H */

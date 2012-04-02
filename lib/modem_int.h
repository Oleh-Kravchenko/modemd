#ifndef __MODEM_INTERNAL_H

#include "modem/types.h"

modem_info_t* usb_device_get_info(const char* port);

modem_info_t* modem_find_first(DIR **dir);

modem_info_t* modem_find_next(DIR **dir);

#endif /* __MODEM_INTERNAL_H */

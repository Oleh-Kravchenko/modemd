CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(libmodem)

SET(PROJECT_SOURCES
modem_str.c
rpc.c
rpc.h
)

ADD_LIBRARY(modem SHARED ${PROJECT_SOURCES}
modem.c
)

ADD_LIBRARY(modem_int STATIC ${PROJECT_SOURCES}
modem_db.c
modem_db.h
modem_int.c
queue.h
queue.c
utils/re.c
utils/str.c
utils/file.c
utils/sysfs.c
utils/event.h
utils/event.c
at/at_query.c
at/at_query.h
at/at_utils.c
at/at_utils.h
at/at_common.c
at/at_common.h
at/at_queue.c
at/at_queue.h
hw/hw_common.c
hw/hw_common.h
modems/modem_conf.c
modems/modem_conf.h
modems/mc77x0/registration.c
modems/mc77x0/registration.h
)

SET_TARGET_PROPERTIES(modem PROPERTIES DEFINE_SYMBOL __LIBMODEM_EXPORTS)

INSTALL(TARGETS modem DESTINATION lib)

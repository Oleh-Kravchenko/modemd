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
modem_info.c
modem_info.h
modem_int.c
queue.h
queue.c
utils/re.c
utils/str.c
utils/file.c
utils/sysfs.c
utils/event.h
utils/event.c
proto/proto.h
proto/proto.c
proto/at/at_query.c
proto/at/at_query.h
proto/at/at_utils.c
proto/at/at_utils.h
proto/at/at_common.c
proto/at/at_common.h
proto/at/at_queue.c
proto/at/at_queue.h
hw/hw_common.c
hw/hw_common.h
modems/modem_conf.c
modems/modem_conf.h
modems/mc77x0/at_func.c
modems/mc77x0/at_func.h
modems/mc77x0/registration.c
modems/mc77x0/registration.h
)

SET_TARGET_PROPERTIES(modem PROPERTIES DEFINE_SYMBOL __LIBMODEM_EXPORTS)

INSTALL(TARGETS modem DESTINATION lib)

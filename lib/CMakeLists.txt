CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(libmodem)

SET(PROJECT_SOURCES
include/modem/modem_str.h
modem_str.c
rpc.c
rpc.h
)

ADD_LIBRARY(modem SHARED ${PROJECT_SOURCES}
modem.c
)

ADD_LIBRARY(modem_int STATIC ${PROJECT_SOURCES}
include/modem/modem.h
include/modem/types.h
modem_int.c
modem_int.h
utils.h
utils.c
)

SET_TARGET_PROPERTIES(modem PROPERTIES DEFINE_SYMBOL __LIBMODEM_EXPORTS)

#INSTALL(TARGETS modem DESTINATION lib)
#INSTALL(DIRECTORY include/modem DESTINATION include FILES_MATCHING PATTERN "*.h")

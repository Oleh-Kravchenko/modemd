CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(modem_cli)

ADD_DEFINITIONS(-DCELLULAR_EXPORTS)

SET(PROJECT_SOURCES
../lib/include/modem/modem.h
../lib/include/modem/types.h
main.c
)

ADD_EXECUTABLE(modem_cli ${PROJECT_SOURCES})

TARGET_LINK_LIBRARIES(modem_cli modem)

#INSTALL(TARGETS modem_cli DESTINATION bin)

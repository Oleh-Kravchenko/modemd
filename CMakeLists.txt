CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(modemd)

INCLUDE_DIRECTORIES(lib/include)

SET(PROJECT_SOURCES
lib/include/modem/modem.h
lib/include/modem/types.h
thread.c
thread.h
main.c
)

ADD_EXECUTABLE(modemd ${PROJECT_SOURCES})

TARGET_LINK_LIBRARIES(modemd modem_int pthread)

ADD_SUBDIRECTORY(cli modem_cli)
ADD_SUBDIRECTORY(lib libmodem)

#INSTALL(TARGETS modemd DESTINATION bin)

#CONFIGURE_FILE(
#"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
#"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#IMMEDIATE @ONLY
#)

#ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}"
#-P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#)

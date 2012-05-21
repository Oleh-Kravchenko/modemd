CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(modemd)

SET(PROJECT_SOURCES
thread.c
thread.h
main.c
conf.h
conf.c
)

ADD_EXECUTABLE(modemd ${PROJECT_SOURCES})

TARGET_LINK_LIBRARIES(modemd modem_int pthread)

INSTALL(TARGETS modemd DESTINATION bin)

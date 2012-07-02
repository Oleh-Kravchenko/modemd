CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(modemd)

ADD_DEFINITIONS(-D_GNU_SOURCE -D_XOPEN_SOURCE=600 -std=gnu99 -pedantic -Werror -D_DEV_EDITION)

INCLUDE_DIRECTORIES(include
source/libmodem
source/libmodem/proto
source/libmodem/utils)

ADD_SUBDIRECTORY(source/libmodem)
ADD_SUBDIRECTORY(source/modemd)
ADD_SUBDIRECTORY(source/modemd_cli)

INSTALL(DIRECTORY include/modem
DESTINATION include
FILES_MATCHING PATTERN "*.h" 
PATTERN ".svn" EXCLUDE)

CONFIGURE_FILE(
"${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
"${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
IMMEDIATE @ONLY
)

ADD_CUSTOM_TARGET(uninstall "${CMAKE_COMMAND}"
-P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)

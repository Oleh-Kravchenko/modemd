ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_O_TARGET=at.o

JMK_CFLAGS=-I$(JMK_ROOT)/pkg/modemd/source/libmodem

JMK_O_OBJS = \
    at_query.o \
    at_utils.o \
    at_common.o \
    at_queue.o \


include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

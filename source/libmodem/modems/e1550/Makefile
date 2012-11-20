ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_O_TARGET=e1550.o

JMK_CFLAGS=-I$(JMK_ROOT)/pkg/modemd/source/libmodem -I$(JMK_ROOT)/pkg/modemd/source/libmodem/proto

JMK_O_OBJS=at_func.o

include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_O_TARGET=modems.o

JMK_SUBDIRS=mc77x0 e1550

JMK_CFLAGS=-I$(JMK_ROOT)/pkg/modemd/source/libmodem -I$(JMK_ROOT)/pkg/modemd/source/libmodem/proto

JMK_O_OBJS = modem_conf.o registration.o \
    mc77x0/mc77x0.o \
    e1550/e1550.o

include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

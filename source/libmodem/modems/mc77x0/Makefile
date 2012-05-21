ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_O_TARGET=mc77x0.o

JMK_CFLAGS=-I$(JMK_ROOT)/pkg/modemd/source/libmodem

JMK_O_OBJS = registration.o

include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_O_TARGET=protocols.o

JMK_SUBDIRS=at qcqmi

JMK_CFLAGS=-I$(JMK_ROOT)/pkg/modemd/source/libmodem

JMK_O_OBJS = proto.o \
    at/at.o \
    qcqmi/qcqmi.o

JMK_CFLAGS+=-D__QCQMI

include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

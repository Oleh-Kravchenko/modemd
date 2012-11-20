ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_O_TARGET=qcqmi.o

JMK_CFLAGS=-I$(JMK_ROOT)/pkg/modemd/source/libmodem -I$(JMK_ROOT)/pkg/modemd/source/libmodem/proto -D_GNU_SOURCE

JMK_O_OBJS = \
    qcqmi_queue.o \
    qcqmi_common.o \


include $(JMK_ROOT)/pkg/modemd/modemd.mak
include $(JMK_ROOT)/pkg/slqs/slqs.mak

$(call JMKE_INCLUDE_RULES)

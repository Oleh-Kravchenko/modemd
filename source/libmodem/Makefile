ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SO_TARGET=libmodem.so
JMK_A_TARGET=libmodem_int.a

JMK_SUBDIRS=proto hw modems utils

JMK_RAMDISK_LIB_FILES=$(JMK_SO_TARGET)

JMK_CFLAGS=-I$(JMK_ROOT)/pkg/modemd/source/libmodem/proto -D__QCQMI

OBJS=rpc.o modem_str.o

JMK_O_OBJS_$(JMK_SO_TARGET)=$(OBJS) modem.o

JMK_O_OBJS_$(JMK_A_TARGET)=$(OBJS) \
    queue.o \
    modem_db.o \
    modem_int.o \
    hw/hw.o \
    utils/utils.o \
    modems/modems.o \
    proto/protocols.o \

include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

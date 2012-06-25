ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SO_TARGET=libmodem.so
JMK_A_TARGET=libmodem_int.a

JMK_SUBDIRS=at qcqmi hw modems utils

JMK_RAMDISK_LIB_FILES=$(JMK_SO_TARGET)

OBJS=rpc.o modem_str.o

JMK_O_OBJS_$(JMK_SO_TARGET)=$(OBJS) modem.o

JMK_O_OBJS_$(JMK_A_TARGET)=$(OBJS) \
    modem_int.o \
    modem_db.o \
    modem_int.o \
    queue.o \
    utils/utils.o \
    at/at.o \
    hw/hw.o \
    qcqmi/qcqmi.o \
    modems/modem_conf.o \
    modems/mc77x0/registration.o

include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

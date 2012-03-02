ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_EXPORT_HEADERS+=include/modem/modem.h
JMK_EXPORT_HEADERS+=include/modem/types.h


JMK_SO_TARGET=libmodem.so
JMK_RAMDISK_LIB_FILES+=$(JMK_SO_TARGET)

JMK_A_TARGET=libmodem_int.a

JMK_CFLAGS+=-I$(JMK_ROOT)/pkg/modemd/lib/include

OBJS=rpc.o

JMK_CFLAGS_$(JMK_SO_TARGET)+=-D__LIBMODEM_EXPORTS

JMK_O_OBJS_$(JMK_SO_TARGET)=$(OBJS) modem.o
JMK_O_OBJS_$(JMK_A_TARGET)=$(OBJS) modem_int.o utils.o

$(call JMKE_INCLUDE_RULES)

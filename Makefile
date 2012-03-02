ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SUBDIRS+=lib
JMK_SUBDIRS+=cli
#JMK_SUBDIRS+=test

JMK_TARGET=modemd
JMK_RAMDISK_BIN_FILES+=$(JMK_TARGET)

JMK_CFLAGS+=-I$(JMK_ROOT)/pkg/modemd/lib/include

JMK_O_OBJS+=main.o thread.o

JMK_LIBS+=$(JMKE_BUILDDIR)/pkg/modemd/lib/libmodem_int.a __local_pthread

$(call JMKE_INCLUDE_RULES)

ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_TARGET=modem_cli
JMK_RAMDISK_BIN_FILES+=$(JMK_TARGET)

JMK_CFLAGS+=-I$(JMK_ROOT)/pkg/modemd/lib/include

JMK_O_OBJS+=main.o

JMK_LIBS+=$(JMKE_BUILDDIR)/pkg/modemd/lib/libmodem.so

$(call JMKE_INCLUDE_RULES)

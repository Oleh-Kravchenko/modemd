ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_TARGET=modemd
JMK_RAMDISK_BIN_FILES+=$(JMK_TARGET)

JMK_O_OBJS=main.o thread.o conf.o

JMK_LIBS=$(JMKE_BUILDDIR)/pkg/modemd/source/libmodem/libmodem_int.a __local_pthread

JMK_LIBS+=$(JMKE_BUILDDIR)/pkg/slqs/pkgs/libslqsqmi.a \
    $(JMKE_BUILDDIR)/pkg/slqs/pkgs/libslqsapi.a \
    $(JMKE_BUILDDIR)/pkg/slqs/pkgs/libslqscore.a __local_pthread

JMK_CFLAGS+=-I$(JMK_ROOT)/pkg/modemd/source/libmodem

include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

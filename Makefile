ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SUBDIRS+=lib cli

JMK_TARGET=modemd
JMK_RAMDISK_BIN_FILES+=$(JMK_TARGET)

JMK_CFLAGS_$(JMK_TARGET)=-D__HW_C1KMBR -D__MODEMD_DEBUG

JMK_O_OBJS=main.o thread.o mc7700.o queue.o lattice.o hardware.o

JMK_LIBS=$(JMKE_BUILDDIR)/pkg/modemd/lib/libmodem_int.a __local_pthread

include $(JMK_ROOT)/pkg/modemd/modemd.mak
$(call JMKE_INCLUDE_RULES)

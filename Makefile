ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_SUBDIRS+=lib
JMK_SUBDIRS+=cli


#JMK_INTERNAL_HEADERS+=mgt_utils.h serial.h default.h sierra_utils.h 3g_profile.h
#JMK_EXPORT_HEADERS+=dev_sierra_directip.h

JMK_TARGET=cellulard
JMK_RAMDISK_BIN_FILES+=$(JMK_TARGET)

JMK_LIBS+=$(JMKE_BUILDDIR)/pkg/util/libmgt_client.a $(OPENRG_LIBS) $(MGT_LIBS) __local_pthread

JMK_O_OBJS=main.o thread.o lib/libcellulard.o lib/modem.o lib/functions.o lib/rpc.o

$(call JMKE_INCLUDE_RULES)

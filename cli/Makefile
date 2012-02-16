ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_TARGET=cellulard_cli
JMK_RAMDISK_BIN_FILES+=$(JMK_TARGET)

JMK_INTERNAL_HEADERS+=modem.h
JMK_O_OBJS=main.o

JMK_LIBS+=$(JMKE_BUILDDIR)/pkg/util/libmgt_client.a $(OPENRG_LIBS) $(MGT_LIBS)

$(call JMKE_INCLUDE_RULES)

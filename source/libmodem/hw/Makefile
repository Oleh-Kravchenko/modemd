ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_O_TARGET=hw.o

JMK_CFLAGS+=-D__HW_C1KMBR

JMK_O_OBJS = hw_common.o \
    hw_c1kmbr.o

include $(JMK_ROOT)/pkg/modemd/modemd.mak

$(call JMKE_INCLUDE_RULES)

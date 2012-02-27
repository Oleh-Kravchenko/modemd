ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_INTERNAL_HEADERS+=modem.h

JMK_A_TARGET=libcellulard.a
JMK_CFLAGS_$(JMK_A_TARGET)=-DCELLULARD_EXPORTS
JMK_O_OBJS_$(JMK_A_TARGET)=libcellulard.o modem.o functions.o rpc.o
JMK_EXPORT_LIBS+=$(JMK_A_TARGET)

$(call JMKE_INCLUDE_RULES)

ifndef JMKE_ENV_INCLUDED
  JMK_ROOT=../../
  include $(JMK_ROOT)/jmk/env_root.mak
endif

JMK_EXPORT_HEADERS+=include/modem/modem.h include/modem/types.h include/modem/modem_str.h

JMK_SUBDIRS=source/libmodem source/modemd_cli source/modemd

$(call JMKE_INCLUDE_RULES)

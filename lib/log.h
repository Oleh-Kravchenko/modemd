#ifndef __LOG_H
#define __LOG_H

#include <syslog.h>

#include "../conf.h"

/*------------------------------------------------------------------------*/

void log_init(const char* name);

void log_close(void);

/*------------------------------------------------------------------------*/

#define log_info(...) syslog(LOG_INFO | LOG_LOCAL7, __VA_ARGS__)

#define log_dbg(...)

#define log_emerg(...)

#define log_alert(...)

#define log_crit(...)

#define log_err(...)

#define log_warn(...)

#define log_notice(...)

/*

#define log_dbg(...) syslog(LOG_DEBUG, __VA_ARGS__)

#define log_emerg(...) syslog(LOG_EMERG, __VA_ARGS__)

#define log_alert(...) syslog(LOG_ALERT, __VA_ARGS__)

#define log_crit(...) syslog(LOG_CRIT, __VA_ARGS__)

#define log_err(...) syslog(LOG_ERR, __VA_ARGS__)

#define log_warn(...) syslog(LOG_WARNING, __VA_ARGS__)

#define log_notice(...) syslog(LOG_NOTICE, __VA_ARGS__)

*/

#endif /* __LOG_H */

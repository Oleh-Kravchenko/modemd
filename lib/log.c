#include "log.h"

/*------------------------------------------------------------------------*/

void log_init(const char* name)
{
    openlog(name, LOG_CONS | LOG_PID, LOG_DAEMON);
}

/*------------------------------------------------------------------------*/

void log_close()
{
    closelog();
}

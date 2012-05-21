#ifndef __THREAD_H
#define __THREAD_H

#include <modem/modem.h>

/*------------------------------------------------------------------------*/

typedef struct
{
    int sock;

    modem_t* modem;

    int terminate;
} modemd_client_thread_t;

/*------------------------------------------------------------------------*/

void* ThreadWrapper(void* prm);

#endif /* __THREAD_H */

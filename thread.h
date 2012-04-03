#ifndef __THREAD_H
#define __THREAD_H

#include <dirent.h>

/*------------------------------------------------------------------------*/

typedef struct
{
    int sock;

    DIR *dir;

    char port[0x100];

    int terminate;
} modem_client_thread_t;

/*------------------------------------------------------------------------*/

void* ThreadWrapper(void* prm);

#endif /* __THREAD_H */

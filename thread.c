#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "thread.h"
#include "modem/types.h"
#include "lib/rpc.h"
#include "lib/modem_int.h"

/*-------------------------------------------------------------------------*/

void* ThreadWrapper(void* prm)
{
    cellulard_thread_t* priv = prm;
    modem_info_t *modem;
    packet_t *p;

    while((p = rpc_recv(priv->sock)))
    {
        rpc_print(p);

        if(p->hdr.type != TYPE_QUERY)
        {
            rpc_free(p);
            continue;
        }

        if(strcmp(p->func, "modem_find_first") == 0)
        {
            modem = modem_find_first(&priv->dir);

            p->hdr.type = TYPE_RESPONSE;

            if(modem)
            {
                p->data = (uint8_t*)modem;
                p->hdr.data_len = sizeof(*modem);

                printf("%s %04hx:%04hx %s %s\n", modem->port, modem->id_vendor, modem->id_product, modem->manufacturer, modem->product);
            }
        }
        else if(strcmp(p->func, "modem_find_next") == 0)
        {
            modem = modem_find_next(&priv->dir);

            p->hdr.type = TYPE_RESPONSE;

            if(modem)
            {
                p->data = (uint8_t*)modem;
                p->hdr.data_len = sizeof(*modem);

                printf("%s %04hx:%04hx %s %s\n", modem->port, modem->id_vendor, modem->id_product, modem->manufacturer, modem->product);
            }
        }

        rpc_send(priv->sock, p);
        rpc_print(p);
        rpc_free(p);
    }

    /* cleanup resources */
    close(priv->sock);

    if(priv->dir)
        closedir(priv->dir);

    free(priv);

    return(NULL);
}

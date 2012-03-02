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

/*------------------------------------------------------------------------*/

void* ThreadWrapper(void* prm)
{
    cellulard_thread_t* priv = prm;
    modem_info_t *mi;
    rpc_packet_t *p_in = NULL, *p_out = NULL;

    while((p_in = rpc_recv(priv->sock)))
    {
        rpc_print(p_in);

        if(p_in->hdr.type != TYPE_QUERY)
        {
            rpc_free(p_in);
            continue;
        }

        if(strcmp(p_in->func, "modem_find_first") == 0)
        {
            mi = modem_find_first(&priv->dir);

            if(mi)
            {
                p_out = rpc_create(TYPE_RESPONSE, "modem_find_first", (uint8_t*)mi, sizeof(*mi));

                printf("%s %04hx:%04hx %s %s\n", mi->port, mi->id_vendor, mi->id_product, mi->manufacturer, mi->product);
            }
            else
                p_out = rpc_create(TYPE_RESPONSE, "modem_find_first", 0, 0);
             
            free(mi);
        }
        else if(strcmp(p_in->func, "modem_find_next") == 0)
        {
            mi = modem_find_next(&priv->dir);

            if(mi)
            {
                p_out = rpc_create(TYPE_RESPONSE, "modem_find_next", (uint8_t*)mi, sizeof(*mi));

                printf("%s %04hx:%04hx %s %s\n", mi->port, mi->id_vendor, mi->id_product, mi->manufacturer, mi->product);
            }
            else
                p_out = rpc_create(TYPE_RESPONSE, "modem_find_next", 0, 0);

            free(mi);
        }

        rpc_send(priv->sock, p_out);
        rpc_print(p_out);

        rpc_free(p_in);
        rpc_free(p_out);
    }

    /* cleanup resources */
    close(priv->sock);

    if(priv->dir)
        closedir(priv->dir);

    free(priv);

    return(NULL);
}

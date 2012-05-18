#include <stdio.h>
#include <string.h>

#include "hw_common.h"

#ifdef __HW_C1KMBR
    #include "hw_c1kmbr.h"
#endif /* __HW_C1KMBR */

/*------------------------------------------------------------------------*/

void port_power(const char* port, int state)
{
#ifdef __HW_C1KMBR
    if(strcmp(port, "1-1") == 0)
        hw_c1kmbr_ctl(state ? LATTICE_ON_PWRONn_C3 : LATTICE_OFF_PWRONn_C3, NULL);
    else
#endif /* __HW_C1KMBR */
        printf("(WW) Port %s not implemented\n", port);
}

/*------------------------------------------------------------------------*/

void port_reset(const char* port)
{
#ifdef __HW_C1KMBR
    if(strcmp(port, "1-1") == 0)
        hw_c1kmbr_ctl(LATTICE_RST_OFF_ON_PWR_C3, NULL);
    else
#endif /* __HW_C1KMBR */
        printf("(WW) Port %s not implemented\n", port);
}

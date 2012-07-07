#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef __HW_C1KMBR
#   include "hw_c1kmbr.h"
#endif /* __HW_C1KMBR */

/*------------------------------------------------------------------------*/

void port_power(const char* port, int state)
{
	printf("(DD) %s(%s, %d)\n", __func__, port, state);

#ifdef __HW_C1KMBR
	if(strcmp(port, "1-1") == 0)
		hw_c1kmbr_ctl(state ? LATTICE_ON_PWRONn_C3 : LATTICE_OFF_PWRONn_C3, NULL);
	else
#else
	if(strcmp(port, "1-1") == 0)
		system(state ? "TriLed set green" : "TriLed set off");
	else
#endif /* __HW_C1KMBR */
		printf("(WW) %s() Port %s not implemented\n", __func__, port);
}

/*------------------------------------------------------------------------*/

void port_reset(const char* port)
{
	printf("(DD) %s(%s)\n", __func__, port);

#ifdef __HW_C1KMBR
	if(strcmp(port, "1-1") == 0)
		hw_c1kmbr_ctl(LATTICE_RST_OFF_ON_PWR_C3, NULL);
	else
#else
	if(strcmp(port, "1-1") == 0)
	{
		system("TriLed set off");
		system("TriLed set green");
	}
	else
#endif /* __HW_C1KMBR */
		printf("(WW) %s() Port %s not implemented\n", __func__, port);
}

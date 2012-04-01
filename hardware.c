#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "hardware.h"
#include "lattice.h"

void init_port(const char* port)
{
#ifdef __HW_C1KMBR
	if(strcmp(port, "1-1") == 0)
        its_cell_ctrl_lattice(LATTICE_RST_OFF_ON_PWR_C3, NULL);
    else
        printf("(WW) Invalid port: %s\n", port);
#endif /* __HW_C1KMBR */
}

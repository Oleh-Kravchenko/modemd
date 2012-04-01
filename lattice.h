#ifndef __LATTICE_H
#define __LATTICE_H

#ifdef __HW_C1KMBR
#include <vendor/its/its_lattice.h>
#endif /* __HW_C1KMBR */

int its_cell_ctrl_lattice(int cmd, void* arg);

#endif /* __LATTICE_H */

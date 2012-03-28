#ifndef __LATTICE_H
#define __LATTICE_H

#ifdef CONFIG_RG_HW
#include <vendor/its/its_lattice.h>
#endif /* CONFIG_RG_HW */

int its_cell_ctrl_lattice(int cmd, void* arg);

#endif /* __LATTICE_H */

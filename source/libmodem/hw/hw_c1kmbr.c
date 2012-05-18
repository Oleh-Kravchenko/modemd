#include <fcntl.h>
#include <net/if.h>
#include <unistd.h>

#include <kos/knet.h>

#include "lattice.h"

/*------------------------------------------------------------------------*/

int hw_c1kmbr_ctl(int cmd, void* arg)
{
    int res = -1;
    int fd;

    if((fd = open("/dev/rg_chrdev", O_RDWR | O_NONBLOCK)) == -1)
        goto err;

    /* setup type of device */
    if(ioctl(fd, RG_IOCTL_SIOCSETRGCHRDEVTYPE, KOS_CDT_LATTICE) == -1)
        goto err_ioctl;

    res = ioctl(fd, cmd, arg);

err_ioctl:
    close(fd);

err:
    return(res);
}

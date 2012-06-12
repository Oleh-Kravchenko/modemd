#ifndef __MODEM_ERRNO_H
#define __MODEM_ERRNO_H

/* general CMEE errors */

#define __ME_SIM_PIN			11
#define __ME_SIM_PUK			12
#define __ME_SIM_BUSY			14
#define __ME_SIM_WRONG			15

#define __ME_REG_IN_PROGRESS	10000
#define __ME_MCC_LOCKED			10001
#define __ME_MNC_LOCKED			10002
#define __ME_CCID_LOCKED		10003
#define __ME_MSIN_LOCKED		10004

#define __ME_WRITE_FAILED	20000
#define __ME_READ_FAILED	20001

#endif /* __MODEM_ERRNO_H */

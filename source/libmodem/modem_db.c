#include "modems/mc77x0/registration.h"

#include "modem_db.h"

/*------------------------------------------------------------------------*/

modem_db_device_t modem_db_devices[] = {
	{
		/* Huawei */
		.vendor		= 0x12d1,
		.product	= 0x1001,
		.thread_reg	= mc77x0_thread_reg,
		.iface		= {
			{
				.num	= 0,
				.type	= MODEM_PROTO_AT,
			},
		}
	},
	{
		/* Sierra Wireless, Inc. MC7750 */
		.vendor		= 0x1199,
		.product	= 0x68a2,
		.thread_reg	= mc77x0_thread_reg,
		.iface		= {
			{
				.num	= 3,
				.type	= MODEM_PROTO_AT,
			},
			{
				.num	= 8,
				.type	= MODEM_PROTO_QCQMI,
			},
		}
	},
	{
		/* Sierra Wireless, Inc. MC7700 */
		.vendor 	= 0x1199,
		.product	= 0x68a3,
		.thread_reg	= mc77x0_thread_reg,
		.iface		= {
			{
				.num	= 3,
				.type	= MODEM_PROTO_AT,
			},
		}
	},
};

/*------------------------------------------------------------------------*/

const int modem_db_devices_cnt = ARRAY_SIZE(modem_db_devices);

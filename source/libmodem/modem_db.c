#include "modem_db.h"

#include "at/at_common.h"

#include "modems/mc77x0/at_func.h"
#include "modems/mc77x0/registration.h"

/*------------------------------------------------------------------------*/

modem_db_device_t modem_db_devices[] = {
	{
		/* HUAWEI E1550 */
		.vendor_id	= 0x12d1,
		.product_id	= 0x1001,
		.thread_reg	= mc77x0_thread_reg,
		.iface		= {
			{
				.num	= 0,
				.type	= MODEM_PROTO_AT,
			},
		},
		.functions		= {
			.get_fw_version = at_get_fw_version,
		},
	},
	{
		/* Sierra Wireless MC7700 */
		.vendor		= "Sierra Wireless, Incorporated",
		.product	= "MC7700",
		.vendor_id	= 0x1199,
		.product_id	= 0x68a3,
		.thread_reg	= mc77x0_thread_reg,
		.iface		= {
			{
				.num	= 3,
				.type	= MODEM_PROTO_AT,
			},
		},
		.functions		= {
			.get_fw_version = mc77x0_at_get_fw_version,
		},
	},
	{
		/* Sierra Wireless MC7750 */
		.vendor		= "Sierra Wireless, Incorporated",
		.product	= "MC7750",
		.vendor_id	= 0x1199,
		.product_id	= 0x68a2,
		.thread_reg	= mc77x0_thread_reg,
		.iface		= {
			{
				.num	= 3,
				.type	= MODEM_PROTO_AT,
			},
#if 0
			{
				.num	= 8,
				.type	= MODEM_PROTO_QCQMI,
			},
#endif
		},
		.functions		= {
			.get_fw_version = mc77x0_at_get_fw_version,
		},
	},
};

/*------------------------------------------------------------------------*/

const int modem_db_devices_cnt = ARRAY_SIZE(modem_db_devices);

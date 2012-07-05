#include "modem_db.h"

#include "modems/mc77x0/registration.h"

/*------------------------------------------------------------------------*/

modem_db_device_t modem_db_devices[] = {
	{
		/* Huawei */
		.vendor_id	= 0x12d1,
		.product_id	= 0x1001,
		.thread_reg	= mc77x0_thread_reg,
		.iface		= {
			{
				.num	= 0,
				.type	= MODEM_PROTO_AT,
			},
		},
	},
	{
		/* Sierra Wireless, Inc. MC7700 */
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
	},
	{
		/* Sierra Wireless, Inc. MC7750 */
		.product	= "MC7750",
		.vendor_id	= 0x1199,
		.product_id	= 0x68a2,
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
		},
	},
	{
		/* Sierra Wireless, Inc. MC8790V */
		.product	= "MC8790V",
		.vendor_id	= 0x1199,
		.product_id	= 0x68a3,
		.thread_reg	= mc77x0_thread_reg,
		.func		= {
			.modem_get_imei				= NULL,
			.modem_get_signal_quality	= NULL,
			.modem_get_network_time		= NULL,
			.modem_get_imsi				= NULL,
			.modem_get_operator_name	= NULL,
			.modem_network_registration	= NULL,
			.modem_get_network_type		= NULL,
			.modem_change_pin			= NULL,
			.modem_get_fw_version		= NULL,
			.modem_operator_scan		= NULL,
			.modem_get_cell_id			= NULL,
		},
		.iface		= {
			{
				.num	= 3,
				.type	= MODEM_PROTO_AT,
			},
		},
	},
};

/*------------------------------------------------------------------------*/

const int modem_db_devices_cnt = ARRAY_SIZE(modem_db_devices);

#include "modem_info.h"

#include "at/at_common.h"

#include "qcqmi/qcqmi_common.h"

#include "modems/mc77x0/at_func.h"
#include "modems/mc77x0/registration.h"

/*------------------------------------------------------------------------*/

modem_db_device_t modem_db_devices[] = {
#if 0
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
		},
	},
#endif
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
			{
				.num	= 8,
				.type	= MODEM_PROTO_QCQMI,
			},
		},
		.functions		= {
			.get_fw_version			= mc77x0_at_get_fw_version,
			.get_imsi				= qcqmi_get_imsi,
			.get_imei				= qcqmi_get_imei,
			.get_network_time		= qcqmi_get_network_time,
			.get_signal_quality		= qcqmi_get_signal_quality,
			.get_cell_id			= qcqmi_get_cell_id,
			.get_network_type		= qcqmi_get_network_type,
			.network_registration	= qcqmi_network_registration,
			.get_operator_name		= qcqmi_get_operator_name,
			.operator_scan			= qcqmi_operator_scan,
			.operator_select		= qcqmi_operator_select,
			.get_operator_number	= qcqmi_get_operator_number,
		},
	},
};

/*------------------------------------------------------------------------*/

const int modem_db_devices_cnt = ARRAY_SIZE(modem_db_devices);

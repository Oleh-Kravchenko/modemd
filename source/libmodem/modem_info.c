#include "modem_info.h"

#include "at/at_common.h"
#include "qcqmi/qcqmi_common.h"

#include "modems/registration.h"

#include "modems/e1550/at_func.h"
#include "modems/mc77x0/at_func.h"

/*------------------------------------------------------------------------*/

modem_info_device_t modem_info_devices[] = {
	{
		/* HUAWEI E1550 */
		.vendor_id	= 0x12d1,
		.product_id	= 0x1001,
		.thread_reg	= mc77x0_thread_reg,
		.iface		= {
#if 0
			{
				.num	= 0,
				.type	= MODEM_PROTO_AT_PPP,
			},
			{
				.num	= 1,                 /* PCM 8 Khz 16 bit signed little endian */
				.type	= MODEM_PROTO_VOICE, /* 115200 8N1 HFCN, SFCN */
			},
#endif
			{
				.num	= 2,
				.type	= MODEM_PROTO_AT,
			},
		},
		.functions		= {
			.get_fw_version			= at_get_fw_version,
			.get_imsi				= at_get_imsi,
			.get_imei				= at_get_imei,
			.get_signal_quality		= at_get_signal_quality,
			.get_network_type		= e1550_at_get_network_type,
			.network_registration	= at_network_registration,
			.get_operator_name		= at_get_operator_name,
			.operator_scan			= at_operator_scan,
			.operator_select		= at_operator_select,
			.get_operator_number	= at_get_operator_number,
			.cpin_state				= at_cpin_state,
			.cpin_pin				= at_cpin_pin,
			.cpin_puk				= at_cpin_puk,
			.get_freq_bands			= e1550_at_get_freq_bands,
			.get_freq_band			= e1550_at_get_freq_band,
			.set_freq_band			= e1550_at_set_freq_band,
			.ussd_cmd				= e1550_at_ussd_cmd,
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
			.get_fw_version			= mc77x0_at_get_fw_version,
			.get_imsi				= at_get_imsi,
			.get_imei				= at_get_imei,
			.get_network_time		= mc77x0_at_get_network_time,
			.get_signal_quality		= mc77x0_at_get_signal_quality,
			.get_cell_id			= mc77x0_at_get_cell_id,
			.get_network_type		= mc77x0_at_get_network_type,
			.network_registration	= at_network_registration,
			.get_operator_name		= at_get_operator_name,
			.operator_scan			= at_operator_scan,
			.operator_select		= at_operator_select,
			.get_operator_number	= at_get_operator_number,
			.cpin_state				= at_cpin_state,
			.cpin_pin				= at_cpin_pin,
			.cpin_puk				= at_cpin_puk,
			.get_freq_bands			= mc77x0_at_get_freq_bands,
			.get_freq_band			= mc77x0_at_get_freq_band,
			.set_freq_band			= mc77x0_at_set_freq_band,
			.ussd_cmd				= at_ussd_cmd,

			/* data session */
			.set_wwan_profile		= at_set_wwan_profile,
			.start_wwan				= mc77x0_at_start_wwan,
			.stop_wwan				= mc77x0_at_stop_wwan,
			.state_wwan				= mc77x0_at_state_wwan,
		},
	},
#ifdef __QCQMI
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
			.get_fw_version			= qcqmi_get_fw_version,
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
			.cpin_state				= qcqmi_cpin_state,
			.cpin_pin				= qcqmi_cpin_pin,
			.cpin_puk				= qcqmi_cpin_puk,
			.get_freq_bands			= mc77x0_at_get_freq_bands,
			.get_freq_band			= mc77x0_at_get_freq_band,
			.set_freq_band			= mc77x0_at_set_freq_band,

			/* data session */
			.set_wwan_profile		= qcqmi_set_wwan_profile,
			.start_wwan				= qcqmi_start_wwan,
			.stop_wwan				= qcqmi_stop_wwan,
			.state_wwan				= qcqmi_state_wwan,
		},
	},
#endif /* __QCQMI */
};

/*------------------------------------------------------------------------*/

const int modem_info_devices_cnt = ARRAY_SIZE(modem_info_devices);

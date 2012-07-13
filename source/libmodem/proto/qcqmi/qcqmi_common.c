#include <time.h>

#include <SWIWWANCMAPI.h>
#include <qmerrno.h>

#include <modem/types.h>

#include "qcqmi_queue.h"

/*-------------------------------------------------------------------------*/

char* qcqmi_get_imsi(modem_t* modem, char* imsi, size_t len)
{
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	printf("(II) GetIMSI() = %d\n", qcqmi_q->last_error = GetIMSI(len, imsi));

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\timsi = %s\n", imsi);

		return(imsi);
	}

	return(NULL);
}

/*-------------------------------------------------------------------------*/

char* qcqmi_get_imei(modem_t* modem, char* imei, size_t len)
{
	char esn[0x32], meid[0x32];
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	printf("(II) GetSerialNumbers() = %d\n", qcqmi_q->last_error = GetSerialNumbers(sizeof(esn), esn, len, imei, sizeof(meid), meid));

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\timei = %s\n", imei);

		return(imei);
	}

	return(NULL);
}

/*-------------------------------------------------------------------------*/

time_t qcqmi_get_network_time(modem_t* modem)
{
	const unsigned long long ts_gps = 315964800; /* Jan 6, 1980 */
	unsigned long long ts_modem;
	unsigned long ts_src;
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(0);

	/* count of 1.25 ms that have elapsed from the start of GPS time (Jan 6, 1980) */
	printf("(II) GetNetworkTime() = %d\n", qcqmi_q->last_error = GetNetworkTime(&ts_modem, &ts_src));

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\tts_modem = %lld, ts_src = %ld\n", ts_modem, ts_src);

		return(ts_modem * 1.25 / 1000 + ts_gps);
	}

	return(0);
}

/*------------------------------------------------------------------------*/

int qcqmi_get_signal_quality(modem_t* modem, modem_signal_quality_t* sq)
{
	qcqmi_queue_t* qcqmi_q;
	unsigned long radio;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(-1);

	sq->dbm = 0;
	sq->level = 0;

	printf("(II) GetSignalStrength() = %d\n", qcqmi_q->last_error = GetSignalStrength(&sq->dbm, &radio));

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(-1);

	/* calculation signal level */
	sq->level += !!(sq->dbm >= -109);
	sq->level += !!(sq->dbm >= -95);
	sq->level += !!(sq->dbm >= -85);
	sq->level += !!(sq->dbm >= -73);
	sq->level += !!(sq->dbm >= -65);

	printf("\tdbm = %d, level = %d, radio = %ld\n", sq->dbm, sq->level, radio);

	return(0);
}

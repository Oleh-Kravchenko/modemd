#include <time.h>

#include <SWIWWANCMAPI.h>
#include <qmerrno.h>

#include <modem/types.h>

#include "qcqmi_queue.h"

/*-------------------------------------------------------------------------*/

struct QmiNas3GppNetworkInfo
{
	WORD pMCC;
	WORD pMNC;
	ULONG pInUse;
	ULONG pRoaming;
	ULONG pForbidden;
	ULONG pPreferred;
	CHAR pDesription[0xff];
};

/*-------------------------------------------------------------------------*/

char* qcqmi_get_imsi(modem_t* modem, char* imsi, size_t len)
{
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("%s:%d (II) GetIMSI() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = GetIMSI(len, imsi));

	pthread_mutex_unlock(&qcqmi_q->mutex);

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

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("%s:%d (II) GetSerialNumbers() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = GetSerialNumbers(sizeof(esn), esn, len, imei, sizeof(meid), meid));

	pthread_mutex_unlock(&qcqmi_q->mutex);

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

	pthread_mutex_lock(&qcqmi_q->mutex);

	/* count of 1.25 ms that have elapsed from the start of GPS time (Jan 6, 1980) */
	printf("%s:%d (II) GetNetworkTime() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = GetNetworkTime(&ts_modem, &ts_src));

	pthread_mutex_unlock(&qcqmi_q->mutex);

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

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("%s:%d (II) GetSignalStrength() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = GetSignalStrength(&sq->dbm, &radio));

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(-1);

	/* calculation signal level */
	sq->level += !!(sq->dbm >= -109);
	sq->level += !!(sq->dbm >= -95);
	sq->level += !!(sq->dbm >= -85);
	sq->level += !!(sq->dbm >= -73);
	sq->level += !!(sq->dbm >= -65);

	printf("\tdbm = %d, level = %d, radio = %ld\n", sq->dbm, sq->level, radio);

	qcqmi_get_cell_id(modem);

	return(0);
}

/*------------------------------------------------------------------------*/

int qcqmi_get_cell_id(modem_t* modem)
{
	qaQmiServingSystemParam srv;
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(0);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("%s:%d (II) SLQSGetServingSystem() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = SLQSGetServingSystem(&srv));

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(0);

	printf("\tcell id = %d\n", srv.CellID);

	return(srv.CellID);
}

/*------------------------------------------------------------------------*/

char* qcqmi_get_network_type(modem_t* modem, char* network, size_t len)
{
	qcqmi_queue_t* qcqmi_q;
	unsigned long reg_state;
	unsigned long cs_domain;
	unsigned long ps_domain;
	unsigned long ran;
	unsigned long roaming = 0;
	uint8_t network_type[0x32];
	uint8_t network_type_size = sizeof(network_type);
	uint16_t mcc;
	uint16_t mnc;
	char oper_name[0x32];

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("%s:%d (II) GetServingNetwork() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = GetServingNetwork(
			&reg_state,
			&cs_domain,
			&ps_domain,
			&ran,
			&network_type_size,
			network_type,
			&roaming,
			&mcc,
			&mnc,
			sizeof(oper_name),
			oper_name
		)
	);

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(NULL);

	switch(network_type[0])
	{
		case 1:
			strncpy(network, "CDMA 1xRTT", len);
			break;

		case 2:
			strncpy(network, "CDMA 1xEV-DO", len);
			break;

		case 3:
			strncpy(network, "AMPS", len);
			break;

		case 4:
			strncpy(network, "GSM", len);
			break;

		case 5:
			strncpy(network, "UMTS", len);
			break;

		case 6:
			strncpy(network, "WLAN", len);
			break;

		case 7:
			strncpy(network, "GPS", len);
			break;

		case 8:
			strncpy(network, "LTE", len);
			break;

		default:
			return(NULL);
	}

	network[len - 1] = 0;

	printf("\tnetwork_type = %d %s\n", network_type[0], network);

//	printf("\toperator name = %s\n", oper_name);

	return(network);
}

/*------------------------------------------------------------------------*/

modem_network_reg_t qcqmi_network_registration(modem_t* modem)
{
	modem_network_reg_t res = MODEM_NETWORK_REG_UNKNOWN;
	qcqmi_queue_t* qcqmi_q;
	unsigned long reg_state;
	unsigned long cs_domain;
	unsigned long ps_domain;
	unsigned long ran;
	unsigned long roaming = 0;
	uint8_t network_type[0x32];
	uint8_t network_type_size = sizeof(network_type);
	uint16_t mcc;
	uint16_t mnc;
	char oper_name[0x32];

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("%s:%d (II) GetServingNetwork() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = GetServingNetwork(
			&reg_state,
			&cs_domain,
			&ps_domain,
			&ran,
			&network_type_size,
			network_type,
			&roaming,
			&mcc,
			&mnc,
			sizeof(oper_name),
			oper_name
		)
	);

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(res);

	res = reg_state;

	if(roaming)
		res = MODEM_NETWORK_REG_ROAMING;

	printf("\treg_state = %d, roaming = %d, text = %s\n",
		reg_state, roaming, str_network_registration(res));

//	printf("\toperator name = %s\n", oper_name);

	return(res);
}

/*------------------------------------------------------------------------*/

char* qcqmi_get_operator_name(modem_t* modem, char* oper, size_t len)
{
	qcqmi_queue_t* qcqmi_q;
	unsigned long reg_state;
	unsigned long cs_domain;
	unsigned long ps_domain;
	unsigned long ran;
	unsigned long roaming = 0;
	uint8_t network_type[0x32];
	uint8_t network_type_size = sizeof(network_type);
	uint16_t mcc;
	uint16_t mnc;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("%s:%d (II) GetServingNetwork() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = GetServingNetwork(
			&reg_state,
			&cs_domain,
			&ps_domain,
			&ran,
			&network_type_size,
			network_type,
			&roaming,
			&mcc,
			&mnc,
			len,
			oper
		)
	);

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(NULL);

	printf("\toperator name = %s\n", oper);

	return(oper);
}

/*-------------------------------------------------------------------------*/

int qcqmi_operator_scan(modem_t* modem, modem_oper_t** opers)
{
	struct QmiNas3GppNetworkInfo ni[20];
	uint8_t i, nni = ARRAY_SIZE(ni);
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(0);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("%s:%d (II) PerformNetworkScan() = %d\n", __FILE__, __LINE__,
		qcqmi_q->last_error = PerformNetworkScan(&nni, (BYTE*)ni)
	);

	printf("\toperators scanned = %d\n", nni);

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(0);

	if(!(*opers = malloc(sizeof(**opers) * nni)))
		return(0);

	for(i = 0; i < nni; ++ i)
	{
		printf("\tmcc = %d, mnc = %d, InUse = %d, Roaming = %d, Forbidden = %d, Preferred = %d, Description = %s\n",
			ni[i].pMCC, ni[i].pMNC, ni[i].pInUse, ni[i].pRoaming, ni[i].pForbidden, ni[i].pPreferred, ni[i].pDesription);

		(*opers)[i].stat = 1;

		strncpy((*opers)[i].longname, ni[i].pDesription, sizeof((*opers)[i].longname));
		(*opers)[i].longname[sizeof((*opers)[i].longname) -1] = 0;

		strncpy((*opers)[i].shortname, ni[i].pDesription, sizeof((*opers)[i].shortname));
		(*opers)[i].shortname[sizeof((*opers)[i].shortname) -1] = 0;

		snprintf((*opers)[i].numeric, sizeof((*opers)[i].numeric), "%03d%02d", ni[i].pMCC, ni[i].pMNC);

		(*opers)[i].act = 0;
	}

	return(nni);
}

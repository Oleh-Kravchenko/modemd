#include <time.h>
#include <math.h>

#include <modem/types.h>

#include <SWIWWANCMAPI.h>
#include <qmerrno.h>

#include "utils/re.h"

#include "qcqmi_queue.h"

/*-------------------------------------------------------------------------*/

struct QmiNas3GppNetworkInfo
{
	WORD mcc;

	WORD mnc;

	ULONG inuse;

	ULONG roaming;

	ULONG forbidden;

	ULONG preferred;

	CHAR name[0xff];
};

/*------------------------------------------------------------------------*/

static void qcqmi_update_state(modem_t* modem)
{
	qcqmi_queue_t* qcqmi_q;
	uint8_t i;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return;

	pthread_mutex_lock(&qcqmi_q->mutex);

	if(time(NULL) > qcqmi_q->state.next_update)
	{
		qcqmi_q->state.network_type_size = sizeof(qcqmi_q->state.network_type);

		printf("(II) GetServingNetwork() = %d\n",
			qcqmi_q->last_error = GetServingNetwork(
				&qcqmi_q->state.reg_state,
				&qcqmi_q->state.cs_domain,
				&qcqmi_q->state.ps_domain,
				&qcqmi_q->state.ran,
				&qcqmi_q->state.network_type_size,
				qcqmi_q->state.network_type,
				&qcqmi_q->state.roaming,
				&qcqmi_q->state.mcc,
				&qcqmi_q->state.mnc,
				sizeof(qcqmi_q->state.oper_name),
				qcqmi_q->state.oper_name
			)
		);

		if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
		{
			qcqmi_q->state.next_update = time(NULL) + 10;

			printf("\treg_state = %d, cs_domain = %d, ps_domain = %d, ran = %d, roaming = %d, mcc = %d, mnc = %d, oper_name = %s\n",
				qcqmi_q->state.reg_state,
				qcqmi_q->state.cs_domain,
				qcqmi_q->state.ps_domain,
				qcqmi_q->state.ran,
				qcqmi_q->state.roaming,
				qcqmi_q->state.mcc,
				qcqmi_q->state.mnc,
				qcqmi_q->state.oper_name
			);

			printf("\tnetwork_type length = %d\n", qcqmi_q->state.network_type_size);

			for(i = 0; i < qcqmi_q->state.network_type_size; ++ i)
				printf("\t\tnetwork_type = %d\n", qcqmi_q->state.network_type[i]);
		}
	}

#if 1
	ULONG err;

	unsigned long long band = 0;
	if(SLQSGetBandCapability(&band) == eQCWWAN_ERR_NONE)
		printf("(II) SLQSGetBandCapability() band = 0x%lx\n", band);

    ULONG CurrentChannelTXRate;
    ULONG CurrentChannelRXRate;
    ULONG MaxChannelTXRate;
    ULONG MaxChannelRXRate;

	printf("(II) GetConnectionRate() = %d\n", err = GetConnectionRate(&CurrentChannelTXRate, &CurrentChannelRXRate, &MaxChannelTXRate, &MaxChannelRXRate));

	if(err == eQCWWAN_ERR_NONE)
	{
		printf("\tCurrentChannelTXRate = %ld, CurrentChannelRXRate = %ld, MaxChannelTXRate = %ld, MaxChannelRXRate = %ld\n",
			CurrentChannelTXRate, CurrentChannelRXRate, MaxChannelTXRate, MaxChannelRXRate
		);
	}

    ULONG MaxTXChannelRate;
    ULONG MaxRXChannelRate;
    ULONG DataServiceCapability;
    ULONG SimCapability;
    ULONG RadioIfacesSize = 10;
    BYTE  RadioIfaces[10];
	
	printf("(II) GetDeviceCapabilities() = %d\n", err = GetDeviceCapabilities(
		&MaxTXChannelRate,
		&MaxRXChannelRate,
		&DataServiceCapability,
		&SimCapability,
		&RadioIfacesSize,
		RadioIfaces)
	);

	if(err == eQCWWAN_ERR_NONE)
	{
		printf("\tMaxTXChannelRate = %d, MaxRXChannelRate = %d, DataServiceCapability = %d, SimCapability = %d\n", MaxTXChannelRate, MaxRXChannelRate, DataServiceCapability, SimCapability);
		printf("\tRadioIfacesSize = %d\n", RadioIfacesSize);

		for(i = 0; i < RadioIfacesSize; ++ i)
			printf("\t\tRadioIfaces = %d\n", RadioIfaces[i]);
	}
#endif

	pthread_mutex_unlock(&qcqmi_q->mutex);
}

/*------------------------------------------------------------------------*/

char* qcqmi_get_imsi(modem_t* modem, char* imsi, size_t len)
{
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) GetIMSI() = %d\n",
		qcqmi_q->last_error = GetIMSI(len, imsi));

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\timsi = %s\n", imsi);

		return(imsi);
	}

	return(NULL);
}

/*------------------------------------------------------------------------*/

char* qcqmi_get_imei(modem_t* modem, char* imei, size_t len)
{
	char esn[0x32], meid[0x32];
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) GetSerialNumbers() = %d\n",
		qcqmi_q->last_error = GetSerialNumbers(sizeof(esn), esn, len, imei, sizeof(meid), meid));

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\timei = %s\n", imei);

		return(imei);
	}

	return(NULL);
}

/*------------------------------------------------------------------------*/

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
	printf("(II) GetNetworkTime() = %d\n",
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
	struct slqsSignalStrengthInfo sssi;
	qcqmi_queue_t* qcqmi_q;
	int i;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(-1);

	sq->dbm = 0;
	sq->level = 0;

	memset(&sssi, 0, sizeof(sssi));
	sssi.signalStrengthReqMask = 1;

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) SLQSGetSignalStrength() = %d\n",
		qcqmi_q->last_error = SLQSGetSignalStrength(&sssi));

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(-1);

	printf("\tsssi.rxSignalStrengthListLen = %d\n",
		sssi.rxSignalStrengthListLen);

	sq->dbm =  sssi.rxSignalStrengthList[0].rxSignalStrength;

	for(i = 0; i < sssi.rxSignalStrengthListLen; ++ i)
		printf("\t\tsssi.rxSignalStrengthList[%d].rxSignalStrength = %d\n",
			i, sssi.rxSignalStrengthList[i].rxSignalStrength);

	/* calculation signal level */
	sq->level += !!(sq->dbm >= -109);
	sq->level += !!(sq->dbm >= -95);
	sq->level += !!(sq->dbm >= -85);
	sq->level += !!(sq->dbm >= -73);
	sq->level += !!(sq->dbm >= -65);

	printf("\tdbm = %d, level = %d\n", sq->dbm, sq->level);

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

	printf("(II) SLQSGetServingSystem() = %d\n",
		qcqmi_q->last_error = SLQSGetServingSystem(&srv));

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(0);

    printf("\tSystemID = %d, NetworkID = %d, BasestationID = %d, BasestationLatitude = %d, BasestationLongitude = %d, Lac = %d, CellID = %d\n",
		srv.SystemID,
		srv.NetworkID,
		srv.BasestationID,
		srv.BasestationLatitude,
		srv.BasestationLongitude,
		srv.Lac,
		srv.CellID
	);

	return(srv.CellID);
}

/*------------------------------------------------------------------------*/

char* qcqmi_get_network_type(modem_t* modem, char* network, size_t len)
{
	qcqmi_queue_t* qcqmi_q;
	uint8_t network_type = 0;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	qcqmi_update_state(modem);

	pthread_mutex_lock(&qcqmi_q->mutex);

	if(qcqmi_q->state.network_type_size)
		network_type = qcqmi_q->state.network_type[0];

	pthread_mutex_unlock(&qcqmi_q->mutex);

	switch(network_type)
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

	return(network);
}

/*------------------------------------------------------------------------*/

modem_network_reg_t qcqmi_network_registration(modem_t* modem)
{
	modem_network_reg_t res = MODEM_NETWORK_REG_UNKNOWN;
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	qcqmi_update_state(modem);

	pthread_mutex_lock(&qcqmi_q->mutex);

	switch(qcqmi_q->state.reg_state)
	{
		case 0:
			res = MODEM_NETWORK_REG_FAILED;
			break;

		case 1:
			res = qcqmi_q->state.roaming ? MODEM_NETWORK_REG_HOME : MODEM_NETWORK_REG_ROAMING;
			break;

		case 2:
			res = MODEM_NETWORK_REG_SEARCHING;
			break;

		case 3:
			res = MODEM_NETWORK_REG_DENIED;
			break;

		default:
			res = MODEM_NETWORK_REG_UNKNOWN;
			break;
	}

	pthread_mutex_unlock(&qcqmi_q->mutex);

	return(res);
}

/*------------------------------------------------------------------------*/

char* qcqmi_get_operator_name(modem_t* modem, char* oper, size_t len)
{
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	qcqmi_update_state(modem);

	pthread_mutex_lock(&qcqmi_q->mutex);

	strncpy(oper, qcqmi_q->state.oper_name, len);
	oper[len - 1] = 0;

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(NULL);

	return(oper);
}

/*------------------------------------------------------------------------*/

int qcqmi_operator_scan(modem_t* modem, modem_oper_t** opers)
{
	struct QmiNas3GppNetworkInfo ni[20];
	uint8_t i, nni = ARRAY_SIZE(ni);
	qcqmi_queue_t* qcqmi_q;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(0);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) PerformNetworkScan() = %d\n",
		qcqmi_q->last_error = PerformNetworkScan(&nni, (BYTE*)ni)
	);

	pthread_mutex_unlock(&qcqmi_q->mutex);

	if(qcqmi_q->last_error != eQCWWAN_ERR_NONE)
		return(0);

	printf("\toperators found = %d\n", nni);

	if(!(*opers = malloc(sizeof(**opers) * nni)))
		return(0);

	for(i = 0; i < nni; ++ i)
	{
		printf("\t\tmcc = %d, mnc = %d, InUse = %d, Roaming = %d, Forbidden = %d, Preferred = %d, Description = %s\n",
			ni[i].mcc, ni[i].mnc, ni[i].inuse, ni[i].roaming, ni[i].forbidden, ni[i].preferred, ni[i].name);

		if(ni[i].inuse == 1)
			(*opers)[i].stat = MODEM_OPER_STAT_CURRENT;
		else if(ni[i].forbidden == 1)
			(*opers)[i].stat = MODEM_OPER_STAT_FORBIDDEN;
		else if(ni[i].forbidden == 2)
			(*opers)[i].stat = MODEM_OPER_STAT_AVAILABLE;
		else
			(*opers)[i].stat = MODEM_OPER_STAT_UNKNOWN;

		strncpy((*opers)[i].longname, ni[i].name, sizeof((*opers)[i].longname));
		(*opers)[i].longname[sizeof((*opers)[i].longname) -1] = 0;

		strncpy((*opers)[i].shortname, ni[i].name, sizeof((*opers)[i].shortname));
		(*opers)[i].shortname[sizeof((*opers)[i].shortname) -1] = 0;

		snprintf((*opers)[i].numeric, sizeof((*opers)[i].numeric), "%03d%02d", ni[i].mcc, ni[i].mnc);

		(*opers)[i].act = MODEM_OPER_ACT_GSM;
	}

	return(nni);
}

/*------------------------------------------------------------------------*/

int qcqmi_operator_select(modem_t* modem, int hni, modem_oper_act_t act)
{
	unsigned long reg_type = 1, rat = 0;
	uint16_t mcc = 0, mnc = 0;
	qcqmi_queue_t* qcqmi_q;
	enum eQCWWANError err;
	int mnc_len;

	if(!hni)
		return(0);

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(-1);

	if(hni)
	{
		/* reg_type, mcc, mnc and rat */
		/* mnc length calculation */
		mnc_len = hni > 99999 ? 1000 : 100;

		mcc = hni / mnc_len;
		mnc = hni % mnc_len;

		reg_type = 2;

		/* rat */
		switch(act)
		{
			case MODEM_OPER_ACT_GSM:
				rat = 4;
				break;

			case MODEM_OPER_ACT_UTRAN:
				rat = 5;
				break;

			default:
				rat = 0;
				break;
		}
	}

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) InitiateNetworkRegistration(reg_type = %d, mcc = %d, mnc = %d, rat = %d) = %d\n",
		reg_type, mcc, mnc, rat,
		err = qcqmi_q->last_error = InitiateNetworkRegistration(reg_type, mcc, mnc, rat)
	);

	pthread_mutex_unlock(&qcqmi_q->mutex);

	return(err != eQCWWAN_ERR_NONE);
}

/*------------------------------------------------------------------------*/

char* qcqmi_get_operator_number(modem_t* modem, char* oper_number, size_t len)
{
	qcqmi_queue_t* qcqmi_q;
	char* res = NULL;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(NULL);

	qcqmi_update_state(modem);

	pthread_mutex_lock(&qcqmi_q->mutex);

	if(qcqmi_q->state.mcc && qcqmi_q->state.mnc)
		snprintf(res = oper_number, len, "%03d%02d", qcqmi_q->state.mcc, qcqmi_q->state.mnc);

	pthread_mutex_unlock(&qcqmi_q->mutex);

	return(res);
}

/*------------------------------------------------------------------------*/

int qcqmi_set_wwan_profile(modem_t* modem, modem_data_profile_t* profile)
{
	int res = -1;
	qcqmi_queue_t* qcqmi_q;
	ULONG uauth = profile->auth;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) SetDefaultProfileLTE(%d, \"%s\", \"%s\", \"%s\") = %d\n",
		profile->auth, profile->apn, profile->username, profile->password,
		qcqmi_q->last_error = SetDefaultProfileLTE(0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &uauth, "profile3", profile->apn, profile->username, profile->password));

	res = qcqmi_q->last_error != eQCWWAN_ERR_NONE;

	pthread_mutex_unlock(&qcqmi_q->mutex);

	return(res);
}

/*------------------------------------------------------------------------*/

int qcqmi_start_wwan(modem_t* modem)
{
	int res = -1;
	qcqmi_queue_t* qcqmi_q;

	ULONG PDPType;
	ULONG IPAddressv4;
	ULONG PrimaryDNSv4;
	ULONG SecondaryDNSv4;
	USHORT IPAddressv6;
	USHORT PrimaryDNSv6;
	USHORT SecondaryDNSv6;
	ULONG Authentication;
	CHAR ProfileName[33];
	CHAR apn[101];
	CHAR username[33];
	ULONG failurereason = 0;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) GetDefaultProfileLTE() = %d\n",
		qcqmi_q->last_error = GetDefaultProfileLTE(0, &PDPType, &IPAddressv4,
			&PrimaryDNSv4, &SecondaryDNSv4, &IPAddressv6, &PrimaryDNSv6,
			&SecondaryDNSv6, &Authentication, sizeof(ProfileName),
			ProfileName, sizeof(apn), apn, sizeof(username), username));

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("(II) StartDataSessionLTE(\"%s\", %d, \"%s\") = %d\n",
			apn, Authentication, username,
			qcqmi_q->last_error = StartDataSessionLTE(NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, apn, NULL, NULL, &Authentication,
				username, NULL, &qcqmi_q->sessionid, &failurereason, 0x08));

		printf("\tsessionid = 0x%d, failurereason = 0x%x\n", qcqmi_q->sessionid, failurereason);
	}

	res = qcqmi_q->last_error != eQCWWAN_ERR_NONE;

	pthread_mutex_unlock(&qcqmi_q->mutex);

	return(res);
}

/*------------------------------------------------------------------------*/

int qcqmi_stop_wwan(modem_t* modem)
{
	qcqmi_queue_t* qcqmi_q;
	int res = -1;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	pthread_mutex_lock(&qcqmi_q->mutex);

	if(qcqmi_q->sessionid)
	{
		printf("(II) StopDataSession() = %d\n", qcqmi_q->last_error = StopDataSession(qcqmi_q->sessionid));

		if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
			res = qcqmi_q->sessionid = 0;
	}

	pthread_mutex_unlock(&qcqmi_q->mutex);

	return(res);
}

/*------------------------------------------------------------------------*/

modem_state_wwan_t qcqmi_state_wwan(modem_t* modem)
{
	return(state_wwan);
}

/*------------------------------------------------------------------------*/

modem_cpin_state_t qcqmi_cpin_state(modem_t* modem)
{
	modem_cpin_state_t res = MODEM_CPIN_STATE_UNKNOWN;
	qcqmi_queue_t* qcqmi_q;
	unsigned long VerifyRetriesLeft = 0;
	unsigned long UnblockRetriesLeft = 0;
	unsigned long Status;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) UIMGetPINStatus() = %d\n", qcqmi_q->last_error =
		UIMGetPINStatus(1, &Status, &VerifyRetriesLeft, &UnblockRetriesLeft)
	);

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\tStatus = %d, VerifyRetriesLeft = %d, UnblockRetriesLeft = %d\n",
			Status, VerifyRetriesLeft, UnblockRetriesLeft
		);

		switch(Status)
		{
			/* PIN not initialized */
			case 0:
				res = MODEM_CPIN_STATE_UNKNOWN;
				break;

			/* PIN enabled, not verified */
			case 1:
				res = MODEM_CPIN_STATE_PIN;
				break;

			/* PIN enabled, verified */
			case 2:
				res = MODEM_CPIN_STATE_READY;
				break;

			/* PIN disabled */
			case 3:
				res = MODEM_CPIN_STATE_READY;
				break;

			/* PIN blocked */
			case 4:
				res = MODEM_CPIN_STATE_PUK;
				break;

			/* PIN permanently blocked */
			case 5:

			default:
				res = MODEM_CPIN_STATE_UNKNOWN;
				break;
		}
	}

	pthread_mutex_unlock(&qcqmi_q->mutex);

	printf("\t\tres = %d\n", res);

	return(res);
}

/*------------------------------------------------------------------------*/

int qcqmi_cpin_pin(modem_t* modem, const char* pin)
{
	qcqmi_queue_t* qcqmi_q;
	unsigned long VerifyRetriesLeft = 0;
	unsigned long UnblockRetriesLeft = 0;
	unsigned long Status;
	int res = -1;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) UIMVerifyPIN() = %d\n", qcqmi_q->last_error =
		UIMVerifyPIN(1, pin, &VerifyRetriesLeft, &UnblockRetriesLeft)
	);

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\tVerifyRetriesLeft = %d, UnblockRetriesLeft = %d\n",
			VerifyRetriesLeft, UnblockRetriesLeft
		);

		res = 0;
	}

	pthread_mutex_unlock(&qcqmi_q->mutex);

	return(res);
}

/*------------------------------------------------------------------------*/

int qcqmi_cpin_puk(modem_t* modem, const char* puk, const char* pin)
{
	qcqmi_queue_t* qcqmi_q;
	unsigned long VerifyRetriesLeft = 0;
	unsigned long UnblockRetriesLeft = 0;
	unsigned long Status;
	int res = -1;

	if(!(qcqmi_q = (qcqmi_queue_t*)modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) UIMUnblockPIN() = %d\n", qcqmi_q->last_error =
		UIMUnblockPIN(1, puk, pin, &VerifyRetriesLeft, &UnblockRetriesLeft)
	);

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\tVerifyRetriesLeft = %d, UnblockRetriesLeft = %d\n",
			VerifyRetriesLeft, UnblockRetriesLeft
		);

		res = 0;
	}

	pthread_mutex_unlock(&qcqmi_q->mutex);

	return(res);
}

/*------------------------------------------------------------------------*/

modem_fw_ver_t* qcqmi_get_fw_version(modem_t* modem, modem_fw_ver_t* fw_info)
{
	struct qmifwinfo_s qcqmi_fw;
	modem_fw_ver_t* res = NULL;
	qcqmi_queue_t* qcqmi_q;
	char release[0x100];
	regmatch_t* pmatch;
	size_t nmatch;
	struct tm tm;

	if(!(qcqmi_q = modem_proto_get(modem, MODEM_PROTO_QCQMI)))
		return(res);

	pthread_mutex_lock(&qcqmi_q->mutex);

	printf("(II) SLQSGetFirmwareInfo() = %d\n",
		qcqmi_q->last_error = SLQSGetFirmwareInfo(&qcqmi_fw)
	);

	if(qcqmi_q->last_error == eQCWWAN_ERR_NONE)
	{
		printf("\tmodelid = %s\n", qcqmi_fw.dev.s.modelid_str);
		printf("\tbootversion = %s\n", qcqmi_fw.dev.s.bootversion_str);
		printf("\tappversion = %s\n", qcqmi_fw.dev.s.appversion_str);
		printf("\tsku = %s\n", qcqmi_fw.dev.s.sku_str);
		printf("\tpackageid = %s\n", qcqmi_fw.dev.s.packageid_str);
		printf("\tcarrier = %s\n", qcqmi_fw.dev.s.carrier_str);
		printf("\tpriversion = %s\n", qcqmi_fw.dev.s.priversion_str);

		/* parsing firmware release date and version */
		if(!re_parse(qcqmi_fw.dev.s.appversion_str, "(SWI.*) .* .* ([0-9,/]+ [0-9,:]+)", &nmatch, &pmatch))
		{
			re_strncpy(fw_info->firmware, sizeof(fw_info->firmware), qcqmi_fw.dev.s.appversion_str, pmatch + 1);
			re_strncpy(release, sizeof(release), qcqmi_fw.dev.s.appversion_str, pmatch + 2);

			strptime(release, "%Y/%m/%d\r\n%H:%M:%S", &tm);
			fw_info->release = mktime(&tm);

			res = fw_info;
		}
	}

	pthread_mutex_unlock(&qcqmi_q->mutex);
	
	return(res);
}

#include <time.h>

#include <modem/types.h>

#include <SWIWWANCMAPI.h>
#include <qmerrno.h>

char* qcqmi_get_imsi(modem_t* modem, char* imsi, size_t len)
{
printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

	if(GetIMSI(len, imsi) == eQCWWAN_ERR_NONE)
		return(imsi);

	return(NULL);
}

/*-------------------------------------------------------------------------*/

char* qcqmi_get_imei(modem_t* modem, char* imei, size_t len)
{
	char esn[0x32], meid[0x32];

printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

	if(GetSerialNumbers(sizeof(esn), esn, len, imei, sizeof(meid), meid) == eQCWWAN_ERR_NONE)
		return(imei);

	return(NULL);
}

/*-------------------------------------------------------------------------*/

time_t qcqmi_get_network_time(modem_t* modem)
{
	const unsigned long long ts_gps = 315964800; /* Jan 6, 1980 */
	unsigned long long ts_modem;
	unsigned long ts_src;

printf("%s:%d %s()\n", __FILE__, __LINE__, __func__);

	/* count of 1.25 ms that have elapsed from the start of GPS time (Jan 6, 1980) */
	if(GetNetworkTime(&ts_modem, &ts_src) == eQCWWAN_ERR_NONE)
		return(ts_modem * 1.25 / 1000 + ts_gps);

	return(0);

}

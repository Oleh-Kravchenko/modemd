#include <modem/types.h>

#include <SWIWWANCMAPI.h>
#include <qmerrno.h>

char* qcqmi_get_imsi(modem_t* modem, char* imsi, size_t len)
{
	if(GetIMSI(len, imsi) == eQCWWAN_ERR_NONE)
		return(imsi);

	return(NULL);
}

/*-------------------------------------------------------------------------*/

char* qcqmi_get_imei(modem_t* modem, char* imei, size_t len)
{
	if(GetSerialNumbers(0, NULL, len, imei, 0, NULL) == eQCWWAN_ERR_NONE)
		return(imei);

	return(NULL);
}

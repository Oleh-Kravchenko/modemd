#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>

#include "modem/modem.h"
#include "modem/modem_str.h"

/*------------------------------------------------------------------------*/

/* default names */
#define MODEMD_NAME	 "modemd"
#define MODEMD_CLI_NAME "modemd_cli"

#define BOOL2STR(i) (i ? "Yes" : "No")

/*------------------------------------------------------------------------*/

const char help[] =
	"Usage:\n"
	MODEMD_CLI_NAME " -h\n"
	MODEMD_CLI_NAME " [-s SOCKET] [-t] -d\n"
	MODEMD_CLI_NAME " [-s SOCKET] [-t] -p PORT\n"
	MODEMD_CLI_NAME " [-s SOCKET] -u USSD -d\n"
	MODEMD_CLI_NAME " [-s SOCKET] -u USSD -p PORT\n\n"
	MODEMD_CLI_NAME " [-s SOCKET] -c COMMAND -d\n"
	MODEMD_CLI_NAME " [-s SOCKET] -c COMMAND -p PORT\n\n"
	"Keys:\n"
	"-h - show this help\n"
	"-s - file socket path (default: /var/run/" MODEMD_NAME ".ctl)\n"
	"-d - detect all known modems\n"
	"-p - modem port, for example 1-1\n"
	"-u - execute USSD command\n"
	"-c - execute AT command\n"
	"-t - perform a standard sequence of commands on modem\n\n"
	"Examples:\n"
	MODEMD_CLI_NAME " -d -c ATI                             - show AT information\n"
	MODEMD_CLI_NAME " -d -c 'AT+CGDCONT=1,\"IP\",\"apn.com\"'   - set apn\n"
	MODEMD_CLI_NAME " -d -c 'AT+CPIN=\"1111\"'                - set pin code\n"
	MODEMD_CLI_NAME " -d -u '*111#'                         - USSD request";

/*------------------------------------------------------------------------*/

static char opt_sock_path[0x100];
static char opt_modem_port[0x100];
static char opt_modem_cmd[0x100];
static char opt_modem_ussd[0x100];
static int opt_detect_modems;
static int opt_modems_test;

/*------------------------------------------------------------------------*/

int conf_read_cmdline(int argc, char** argv)
{
	int param;

	/* receiving default parameters */
	snprintf(opt_sock_path, sizeof(opt_sock_path), "/var/run/%s.ctl", MODEMD_NAME);
	*opt_modem_port = 0;
	*opt_modem_cmd = 0;
	*opt_modem_ussd = 0;
	opt_detect_modems = 0;
	opt_modems_test = 0;

	/* analyze command line */
	while((param = getopt(argc, argv, "hs:dp:c:tu:")) != -1)
	{
		switch(param)
		{
			case 's':
				strncpy(opt_sock_path, optarg, sizeof(opt_sock_path) - 1);
				opt_sock_path[sizeof(opt_sock_path) - 1] = 0;
				break;

			case 'd':
				opt_detect_modems = 1;
				break;

			case 'p':
				strncpy(opt_modem_port, optarg, sizeof(opt_modem_port) - 1);
				opt_modem_port[sizeof(opt_modem_port) - 1] = 0;
				break;

			case 'c':
				strncpy(opt_modem_cmd, optarg, sizeof(opt_modem_cmd) - 1);
				opt_modem_cmd[sizeof(opt_modem_cmd) - 1] = 0;
				break;

			case 't':
				opt_modems_test = 1;
				break;

			case 'u':
				strncpy(opt_modem_ussd, optarg, sizeof(opt_modem_ussd) - 1);
				opt_modem_ussd[sizeof(opt_modem_ussd) - 1] = 0;
				break;

			default: /* '?' */
				puts(help);
				return(1);
		}
	}

	/* check input paramets ... */
	if(!*opt_modem_port && !opt_detect_modems)
	{
		puts(help);
		return(1);
	}

	/* ... and their conficts */
	if(
		(opt_detect_modems && *opt_modem_port) ||
		(opt_modems_test && *opt_modem_cmd) ||
		(opt_modems_test && *opt_modem_ussd) ||
		(*opt_modem_cmd && *opt_modem_ussd)
	)
	{
		puts(help);
		return(1);
	}

	return(0);
}

/*------------------------------------------------------------------------*/

const char* modem_signal_level_str(uint8_t level)
{
	const static char* slevel[] =
	{
		"",
		"#",
		"##",
		"###",
		"####",
		"#####"
	};

	if(level > 5)
		level = 5;

	return(slevel[level]);
}

/*------------------------------------------------------------------------*/

void modem_test(const char* port)
{
	modem_signal_quality_t sq;
	modem_fw_ver_t fw_info;
	const struct tm* tm;
	usb_device_info_t mi;
	char msg[0x100];
	modem_t* modem;
	int cell_id;
	time_t t;

	/* try open modem */
	if(!(modem = modem_open_by_port(port)))
		return;

	/* if modem detected, this printf will be waste */
	if(!opt_detect_modems && modem_get_info(modem, &mi))
		printf("\n      Device: [port: %s] [%04hx:%04hx] [%s %s]\n",
			mi.port, mi.id_vendor, mi.id_product, mi.vendor, mi.product);

#if _DEV_EDITION /* for testing purpose */
	int giveup;

	for(giveup = 30; modem_get_last_error(modem) != -1 && giveup; -- giveup)
	{
		puts("Waiting for modem registration ready..");
		sleep(10);
	}
#endif /* _DEV_EDITION */

	/* show modem info */

	if(modem_get_imei(modem, msg, sizeof(msg)))
		printf("        IMEI: [%s]\n", msg);

	if(modem_get_imsi(modem, msg, sizeof(msg)))
		printf("        IMSI: [%s]\n", msg);

	if(modem_get_operator_name(modem, msg, sizeof(msg)))
		printf("    Operator: [%s]\n", msg);

	if(modem_get_network_type(modem, msg, sizeof(msg)))
		printf("     Network: [%s]\n", msg);

	if(!modem_get_signal_quality(modem, &sq))
		printf("      Signal: [%d] dBm, [%-5s] Level\n", sq.dbm, modem_signal_level_str(sq.level));

	if((t = modem_get_network_time(modem)))
	{
		tm = gmtime(&t);
		strftime(msg, sizeof(msg), "%Y.%m.%d %H:%M:%S", tm);
		printf("  Modem time: %s", asctime(tm));
	}

	printf("Registration: [%s]\n", str_network_registration(modem_network_registration(modem)));

	if(modem_get_fw_version(modem, &fw_info))
	{
		tm = gmtime(&fw_info.release);
		strftime(msg, sizeof(msg), "%Y.%m.%d %H:%M:%S", tm);
		printf("    Firmware: [%s], Release: [%s]\n", fw_info.firmware, msg);
	}

	if((cell_id = modem_get_cell_id(modem)))
		printf("     Cell ID: [%d]\n", cell_id);

#if _DEV_EDITION /* for testing purpose */
	const char wait_bar[] = "|/-\\";
	int nbar = 0;

	if(!modem_operator_scan_start(modem, "/tmp/op_list.conf"))
	{
		printf("    Scanning: [\r");
		fflush(stdout);

		while(modem_operator_scan_is_running(modem) == 1)
		{
			printf("    Scanning: [%c]\r", wait_bar[nbar]);
			fflush(stdout);

			++ nbar;

			if(nbar == sizeof(wait_bar) - 1)
				nbar = 0;

			usleep(200000);
		}

		printf("    Scanning: [/tmp/op_list.conf]\n");
	}
#endif /* _DEV_EDITION */

#if 0
	modem_oper_t *opers = NULL;
	int i;

	puts("Performing operator scan:");

	i = modem_operator_scan(modem, &opers);

	while(i > 0)
	{
		-- i;

		printf("%d,%s,%s,%s,%d\n",
			opers[i].stat, opers[i].longname, opers[i].shortname, opers[i].numeric, opers[i].act);
	}

	free(opers);
#endif /* _DEV_EDITION */

	/* close modem */
	modem_close(modem);
}

/*------------------------------------------------------------------------*/

void print_modem_at_cmd(const char* port, const char* cmd)
{
	modem_t* modem;
	char *answer;

	/* try open modem */
	if(!(modem = modem_open_by_port(port)))
		return;

	answer = modem_at_command(modem, opt_modem_cmd);

	if(answer)
		printf("Answer: [\n%s\n]\n", answer);

	free(answer);

	/* close modem */
	modem_close(modem);
}

/*------------------------------------------------------------------------*/

void print_modem_ussd_cmd(const char* port, const char* cmd)
{
	modem_t* modem;
	char *answer;

	/* try open modem */
	if(!(modem = modem_open_by_port(port)))
		return;

	answer = modem_ussd_cmd(modem, cmd);

	if(answer)
		printf("      Answer: [%s]\n", answer);

	free(answer);

	/* close modem */
	modem_close(modem);
}

/*------------------------------------------------------------------------*/

void modem_do(const char* port)
{
	if(opt_modems_test)
		modem_test(port);

	if(*opt_modem_cmd)
		print_modem_at_cmd(port, opt_modem_cmd);

	if(*opt_modem_ussd)
		print_modem_ussd_cmd(port, opt_modem_ussd);
}

/*------------------------------------------------------------------------*/

int main(int argc, char** argv)
{
	usb_device_info_t mi;
	modem_find_t* find;
	int res = 0;

	if((res = conf_read_cmdline(argc, argv)))
		goto exit;

	/* show configuration */
	printf(
		"     Basename: %s\n"
		"  Socket file: %s\n"
		"         Port: %s\n"
		"      Command: %s\n"
		"Detect modems: %s\n"
		"  Test modems: %s\n",
		argv[0], opt_sock_path, opt_modem_port, opt_modem_cmd,
		BOOL2STR(opt_detect_modems),
		BOOL2STR(opt_modems_test)
	);

	/* initialize modem library */
	if(modem_init(opt_sock_path))
	{
		perror("modem_init()");
		res = errno;
		goto exit;
	}

	if(opt_detect_modems)
	{
		/* let's find some modem's */
		find = modem_find_first(&mi);

		while(find)
		{
			printf("\n      Device: [port: %s] [%04hx:%04hx] [%s %s]\n",
				mi.port, mi.id_vendor, mi.id_product, mi.vendor, mi.product);

			modem_do(mi.port);

			find = modem_find_next(find, &mi);
		}
	}
	else if(*opt_modem_port)
		modem_do(opt_modem_port);

	modem_cleanup();

exit:
	return(res);
}

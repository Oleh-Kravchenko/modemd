#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
#include <sys/types.h>
#include <dirent.h>

/*------------------------------------------------------------------------*/

char* file_get_contents(const char *filename, char* s, const int size)
{
    FILE *f;
    char* res;

    if(!(f = fopen(filename, "r")))
        return(0);

    res = fgets(s, size, f);

    fclose(f);

    int rn = strlen(res);

    /* removing eof */
    if(rn && (res[rn - 1] == '\n' || res[rn - 1] == '\n'))
        res[rn - 1] = 0;

    return(res);
}

/*------------------------------------------------------------------------*/

int file_get_contents_hex(const char* filename)
{
    char hex[256];
    int res = 0;

    if(file_get_contents(filename, hex, sizeof(hex)))
        sscanf(hex, "%x", &res);

    return(res);
}

/*------------------------------------------------------------------------*/

int its_modem(uint16_t vendor, uint16_t product)
{
    return(vendor == 0x1199 && product == 0x68a3);
}

/*------------------------------------------------------------------------*/

int serial_open(const char* portname, int flags)
{
	struct termios tp;
	int port;

	/* send up command to cns port */
	port = open(portname, flags);

	if(port < 0)
		return(-1);

	/* serial port settings */
	tcgetattr(port, &tp);

	/* make sure tp is all blank and no residual values set
	safer than modifying current settings */
	tp.c_lflag = 0; /* implies non-canoical mode */
	tp.c_iflag = 0;
	tp.c_oflag = 0;
	tp.c_cflag = 0;
	tp.c_cflag |= B115200;
	tp.c_cflag |= CS8;

	/* ignore modem lines like hangup */
	tp.c_cflag |= CLOCAL;

	/* let us read from this device! */
	tp.c_cflag |= CREAD;

	/* perform newline mapping, useful when dealing with dos/windows systems */
	tp.c_oflag = OPOST;

	/* pay attention to hangup line */
	tp.c_cflag |= HUPCL;

	/* don't wait between receiving characters */
	tp.c_cc[VMIN] = 1;
	tp.c_cc[VTIME] = 0;

	tcsetattr(port, TCSANOW, &tp);

	return(port);
}

/*------------------------------------------------------------------------*/

char* modem_get_at_port_name(const char* port, char* tty, int tty_len)
{
	uint16_t vendor, product;
	char path[0xff];
	DIR *dir;
    struct dirent *item;
	char* at_port;

	/* reading id's of modem */
	snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idVendor", port);
	vendor = file_get_contents_hex(path);

	snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s/idProduct", port);
	product = file_get_contents_hex(path);

	if((vendor == 0x1199 && product == 0x68a3))
	{
		snprintf(path, sizeof(path), "/sys/bus/usb/devices/%s:1.3/", port);
		
	    if(!(dir = opendir(path)));

		while((item = readdir(dir)))
		{
			if((at_port = strstr(item->d_name, "ttyUSB")))
			{
				snprintf(tty, tty_len - 1, "/dev/%s", at_port);

				return(tty);
			}
		}

	}
	else
		return(NULL);
}

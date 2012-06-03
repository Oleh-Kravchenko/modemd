#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "str.h"

/*------------------------------------------------------------------------*/

int serial_open(const char* tty, int flags)
{
	struct termios tp;
	int fd;

	if((fd = open(tty, flags)) == -1)
		goto err;

	/* serial port settings */
	if(tcgetattr(fd, &tp) == -1)
		goto tc_err;

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

	if(tcsetattr(fd, TCSANOW, &tp) == -1)
		goto tc_err;

	return(fd);

tc_err:
	close(fd);

err:
	return(-1);
}

/*------------------------------------------------------------------------*/

char* file_get_contents(const char *filename, char* s, const int size)
{
	char* res;
	FILE *f;

	if(!(f = fopen(filename, "r")))
		return(NULL);

	res = fgets(s, size, f);

	fclose(f);

	if(res)
		trim(res);

	return(res);
}

/*------------------------------------------------------------------------*/

unsigned int file_get_contents_hex(const char* filename)
{
	char hex[256];
	unsigned int res = 0;

	if(file_get_contents(filename, hex, sizeof(hex)))
		sscanf(hex, "%x", &res);

	return(res);
}

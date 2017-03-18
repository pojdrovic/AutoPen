#include <fcntl.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/stat.h>

#include "error.h"
#include "serial_io.h"

serial_io::serial_io()
{
}

serial_io::serial_io(std::string dev_in, int bps_in) : dev(dev_in)
{
	fd = open(dev.c_str(), O_RDWR);
	if (fd == -1)
		error_exit("Failed to open %s", dev.c_str());

	int bps = -1;
	switch(bps_in)
	{
		case -1: // this is the "do not change the serial settings"-flag
			break;

		case 50:
			bps = B50;
			break;

		case 75:
			bps = B75;
			break;

		case 110:
			bps = B110;
			break;

		case 134:
			bps = B134;
			break;

		case 150:
			bps = B150;
			break;

		case 200:
			bps = B200;
			break;

		case 300:
			bps = B300;
			break;

		case 600:
			bps = B600;
			break;

		case 1200:
			bps = B1200;
			break;

		case 1800:
			bps = B1800;
			break;

		case 2400:
			bps = B2400;
			break;

		case 4800:
			bps = B4800;
			break;

		case 9600:
			bps = B9600;
			break;

		case 19200:
			bps = B19200;
			break;

		case 38400:
			bps = B38400;
			break;

		case 57600:
			bps = B57600;
			break;

		case 115200:
			bps = B115200;
			break;

		case 230400:
			bps = B230400;
			break;

		case 460800:
			bps = B460800;
			break;

		case 500000:
			bps = B500000;
			break;

		case 576000:
			bps = B576000;
			break;

		case 921600:
			bps = B921600;
			break;

		case 1000000:
			bps = B1000000;
			break;

		case 1152000:
			bps = B1152000;
			break;

		case 1500000:
			bps = B1500000;
			break;

		case 2000000:
			bps = B2000000;
			break;

		case 2500000:
			bps = B2500000;
			break;

		case 3000000:
			bps = B3000000;
			break;

		case 3500000:
			bps = B3500000;
			break;

		case 4000000:
			bps = B4000000;
			break;

		default:
			error_exit("baudrate %d is not understood", bps_in);
	}

	if (bps_in != -1)
	{
		struct termios newtio;
		if (tcgetattr(fd, &newtio) == -1)
			error_exit("tcgetattr failed");
		newtio.c_iflag = IGNBRK; // | ISTRIP;
		newtio.c_oflag = 0;
		newtio.c_cflag = bps | CS8 | CREAD | CLOCAL | CSTOPB;
		newtio.c_lflag = 0;
		if (tcsetattr(fd, TCSANOW, &newtio) == -1)
			error_exit("tcsetattr failed");
	}
}

serial_io::~serial_io()
{
	if (fd != -1)
		close(fd);
}

void serial_io::flush()
{
        if (tcflush(fd, TCIFLUSH) == -1)
		error_exit("tcflush failed");
}

int serial_io::read(char *where, size_t n)
{
	return ::read(fd, where, n);
}

int serial_io::write(const char *what, size_t n)
{
	return ::write(fd, what, n);
}

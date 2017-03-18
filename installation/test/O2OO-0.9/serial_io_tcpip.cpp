#include <errno.h>
#include <fcntl.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "error.h"
#include "utils.h"
#include "serial_io.h"
#include "serial_io_tcpip.h"

serial_io_tcpip::serial_io_tcpip(std::string host_in, int port_in) : host(host_in), port(port_in)
{
	fd = -1;

	reconnect();
}

serial_io_tcpip::~serial_io_tcpip()
{
	if (fd != -1)
		close(fd);
}

void serial_io_tcpip::reconnect()
{
	if (fd != -1)
		close(fd);

	fd = connect_to(host.c_str(), port);
	if (fd == -1)
		error_exit("Failed connecting to %s %d", host.c_str(), port);
}

void serial_io_tcpip::flush()
{
	for(;;)
	{
		struct timeval tv = { 0, 1 };
		fd_set rfds;

		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);

		if (select(fd + 1, &rfds, NULL, NULL, &tv) == -1)
		{
			if (errno == EBADF)
				reconnect();
			else if (errno != EINTR)
				error_exit("select failed on socket");

			continue;
		}

		if (!FD_ISSET(fd, &rfds))
			break;

		char buffer[4096];
		::read(fd, buffer, sizeof buffer);
	}
}

int serial_io_tcpip::read(char *where, size_t n)
{
	int rc = -1;

	for(;;)
	{
		rc = ::read(fd, where, n);

		if (rc == -1 && errno == EBADF)
		{
			reconnect();
			continue;
		}

		break;
	}

	return rc;
}

int serial_io_tcpip::write(const char *what, size_t n)
{
	int rc = -1;

	for(;;)
	{
		rc = ::write(fd, what, n);

		if (rc == -1 && errno == EBADF)
		{
			reconnect();
			continue;
		}

		break;
	}

	return rc;
}

// SVN: $Revision: 457 $
#include <errno.h>
#include <iostream>
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>

#include "error.h"
#include "byte_array.h"
#include "terminal.h"
#include "terminal_console.h"
#include "utils.h"
#include "obd2-emulation.h"

bool full_speed = false;

void mysleep(int us)
{
	if (!full_speed)
		usleep(us);
}

bool send(int fd, std::string what)
{
	std::cout << fd << " out: " << what << std::endl;

	if (WRITE(fd, what.c_str(), what.size()) == -1)
	{
		std::cout << " problem transmitting to fd " << fd << std::endl;
		return false;
	}

	return true;
}

bool send(int fd, byte_array *in)
{
	std::string out;

	for(int index=0; index<in -> size(); index++)
	{
		if (out.size() > 0)
			out += " ";

		out += format("%02X", in -> get_uchar(index));
	}

	out += "\r";

	return send(fd, out);
}

bool recv_cmd(int fd, std::string *result)
{
	for(;;)
	{
		char buffer[2] = { 0 };
		int rc = read(fd, buffer, 1);

		if (rc == 0)
			return false;

		if (rc == -1)
		{
			if (errno == EINTR)
				continue;

			return false;
		}

		if (buffer[0] == '\r')
			break;

		*result += buffer;
	}

	return true;
}

bool sleep_stopable(int fd, int sleep_time)
{
	if (full_speed)
		return false;

	struct timeval tv = { 0, sleep_time };
	fd_set rfds;

	FD_ZERO(&rfds);
	FD_SET(fd, &rfds);

	for(;;)
	{
		int rc = select(fd + 1, &rfds, NULL, NULL, &tv);
		if (rc == 0)
			return false;

		if (rc == -1)
		{
			if (errno == EINTR)
				continue;

			error_exit("select failed");
		}

		break;
	}

	std::cout << fd << " command interrupted" << std::endl;

	return true;
}

void clean_str(std::string in, std::string *out, unsigned int *n)
{
	std::vector<std::string> parts = split_string(in, " ");

	std::string last;
	for(unsigned int index=0; index<parts.size(); index++)
	{
		std::string cur = parts.at(index);

		unsigned int pos = 0;
		while(pos < cur.size())
		{
			last = cur.substr(pos, 2);

			if (last.size() == 2)
			{
				if ((*out).size())
					*out += " ";

				*out += last;
			}

			pos += last.size();
			while(pos < cur.size() && cur.substr(pos, 1) == " ")
				pos++;
		}
	}

	if (last.size() == 1)
		*n = atoi(last.c_str());
}

void handle_client(terminal *t, int fd)
{
	disable_nagle(fd);

	mysleep(5000);
	if (!send(fd, "BUS INIT..."))
		return;
	mysleep(125000);
	if (!send(fd, "OK\r"))
		return;

	if (rand() % 100 == 1)
	{
		if (!send(fd, "UNABLE TO CONNECT"))
			return;
	}

	std::string prev_command;
	bool first = true, copy = true;;
	for(;;)
	{
		if (!send(fd, ">"))
			return;

		std::string recv;
		if (!recv_cmd(fd, &recv))
		{
			std::cout << fd << " receive problem" << std::endl;
			return;
		}

		std::cout << fd << " in: " << recv << std::endl;

		recv = trim(recv);

		if (recv.empty())
			recv = prev_command;

		if (copy)
		{
			if (!send(fd, recv + "\r"))
				return;
		}

		if (first)
		{
			if (!send(fd, "SEARCHING...\r"))
				return;

			mysleep(100000);

			first = false;
		}

		if (rand() % 100 < 5)
		{
			if (!send(fd, "CAN ERROR\r"))
				return;

			continue;
		}

		if (recv == "ATZ" || recv == "AT Z")
		{
			mysleep(10000);
			if (!send(fd, "ELM327 v1.5\r"))
				return;
		}
		else if (recv == "ATE0" || recv == "AT E0")
		{
			mysleep(10000);
			if (!send(fd, "OK\r"))
				return;

			copy = false;
		}
		else if (recv == "ATRV" || recv == "AT RV")
		{
			mysleep(21000);
			if (!send(fd, "12.3V\r"))
				return;
		}
		else if (recv.substr(0, 2) == "AT")
		{
			if (!send(fd, "?\r"))
				return;
		}
		else
		{
			int sleep_time = (rand() % 100 + 1) * 1000;
			if (sleep_stopable(fd, sleep_time))
			{
				if (!send(fd, "STOPPED\r"))
					return;

				continue;
			}

			prev_command = recv;

			std::string recv_clean;
			unsigned int n_lines_allowed = 99;
			clean_str(recv, &recv_clean, &n_lines_allowed);

			byte_array *b = new byte_array(recv_clean);

			std::vector<byte_array *> result;
			emulate_obd2(t, b, &result);

			delete b;

			if (result.size() == 0)
			{
				if (!send(fd, "NO DATA\r"))
					return;
			}
			else
			{
				for(unsigned int index=0; index<result.size(); index++)
				{
					if (index < n_lines_allowed && !send(fd, result.at(index)))
						return;

					delete result.at(index);
				}
			}
		}
	}

	std::cerr << "loop end?" << std::endl;
}

void help()
{
	std::cerr << "-p x   TCP port to listen on" << std::endl;
	std::cerr << "-f     process in full speed, no delays" << std::endl;
	std::cerr << "-h     this help" << std::endl;
}

int main(int argc, char *argv[])
{
	std::cout << "O2OO-elm327-emulator v" VERSION ", (C) folkert@vanheusden.com" << std::endl;

	int port = 35000, c;
	while((c = getopt(argc, argv, "p:fh")) != -1)
	{
		switch(c)
		{
			case 'p':
				port = atoi(optarg);
				break;

			case 'f':
				full_speed = true;
				break;

			case 'h':
				help();
				return 0;

			default:
				help();
				return 1;
		}
	}

	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	terminal *t = new terminal_console(3, true, "");

	int fd = start_listen(NULL, port);

	std::cout << "Listening on port " << port << std::endl;
	if (full_speed)
		std::cout << "Full speed mode" << std::endl;

	for(;;)
	{
		int new_socket_fd = accept(fd, NULL, NULL);

		if (new_socket_fd == -1)
			continue;

		std::string endpoint = get_endpoint_name(new_socket_fd);
		std::cout << "Connection made with " << endpoint << " (" << new_socket_fd << ")" << std::endl;

		if (fork() == 0)
		{
			srand(time(NULL) ^ getpid());

			handle_client(t, new_socket_fd);

			std::cout << "Session with " << endpoint << " terminated" << std::endl;
			exit(0);
		}
	}

	return 0;
}

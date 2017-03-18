// SVN: $Revision: 465 $
#include <errno.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <stdio.h>
#include <string.h>

#include "error.h"
#include "utils.h"
#include "byte_array.h"
#include "terminal.h"
#include "serial_io.h"
#include "IO_Device.h"
#include "IO_ELM327.h"
// FIXME make strings static so that we can directly reference the battery voltage symbol
// #include "sensors/sensor.h"
// #include "sensors/io_sensor.h"
// #include "sensors/io_sensor_battery_voltage.h"

IO_ELM327::IO_ELM327(serial_io *sio_in, terminal *t_in, bool full_speed_in) : IO_Device(t_in, full_speed_in), sio(sio_in)
{
	last_msg = 0;

	sleep(1);
	reset_device();
}

void IO_ELM327::reset_device()
{
	sio -> flush();

	std::string init = "ATZ\r";
	sio -> write(init.c_str(), init.size());

	t -> emit(LL_DEBUG_SOME, "Waiting for ATZ reply...");

	for(;;)
	{
		std::string reply = recv();

		t -> emit(LL_DEBUG_ALL, " device says: " + reply);

		if (reply == "")
			continue;

		if (reply.substr(0, 6) == "ELM327")
		{
			t -> emit(LL_DEBUG_ALL, " device ack found");
			dev_info = reply;
			break;
		}

		sio -> write(init.c_str(), init.size());
	}

	t -> emit(LL_DEBUG_SOME, "Flushing serial connection...");
	sleep(1);
	sio -> flush();

	std::string init2 = "ATE0\r";
	sio -> write(init2.c_str(), init2.size());

	t -> emit(LL_DEBUG_SOME, "Waiting for ATE0 reply...");

	for(;;)
	{
		std::string reply = recv();

		t -> emit(LL_DEBUG_ALL, " device says: " + reply);

		if (reply == "OK")
		{
			t -> emit(LL_DEBUG_ALL, " ATE0 ack from device");
			break;
		}
	}

	wait_for_prompt();

	sio -> flush();

	t -> emit(LL_DEBUG_SOME, "Device resetted");
}

IO_ELM327::~IO_ELM327()
{
}

void IO_ELM327::wait_for_prompt()
{
	std::string flushed;
	int err_cnt = 0;

	for(;;)
	{
		char buffer[2] = { 0, 0 };

		int rc = sio -> read(buffer, 1);
		if (rc == 0)
		{
			// see comment in recv()
			usleep(1000);
		}

		if (rc == -1)
		{
			if (errno == EINTR)
				continue;

			if (++err_cnt >= 3)
				error_exit("Read error from ELM327");
		}

		if (buffer[0] == '>')
			break;

		if (buffer[0] != '\r')
			flushed += buffer;
	}

	if (!flushed.empty())
		t -> emit(LL_DEBUG_SOME, "Flushed: " + flushed);
}

std::string IO_ELM327::recv()
{
	bool recv_EOF = false;
	std::string in;

	for(;;)
	{
		char buffer[2] = { 0, 0 };

		int rc = sio -> read(buffer, 1);
		if (rc == 0)
		{
			// some devices frequently cause EOFs while
			// the connection still is there, so just
			// sleep for now and retry the read
			// FIXME: implement a maximum number of
			// retries?
			usleep(1000);
			recv_EOF = true;
		}

		if (rc == -1)
		{
			if (errno == EINTR)
				continue;

			error_exit("Read error from ELM327");
		}

		if (buffer[0] == '\r' || buffer[1] == '\n')
			break;

		// elm327 documentation states that occasionally a 0x00 may be inserted
		if (buffer[0] != 0x00)
			in += buffer;

		if (buffer[0] == '>')
			break;
	}

	if (recv_EOF)
		t -> emit(LL_DEBUG_ALL, "EOFs detected");

	return in;
}

void IO_ELM327::send_recv(byte_array *command, std::vector<byte_array *> *results, int result_count_limit)
{
	slowdown(); // slow down to once every 10ms a message

	std::string hex;
	for(int index=0; index<command -> size(); index++)
		hex += format("%02x ", command -> get_uchar(index));
	hex = trim(hex);

	results -> clear();

	if (result_count_limit >= 1 && result_count_limit < 10)
		hex += " " + format("%d", result_count_limit);
	else if (result_count_limit != -1)
		t -> emit(LL_ERROR, format("ELM327: result limit %d not understood (must be either -1 or 1 <= 1 < 10), ignoring", result_count_limit));

	t -> emit(LL_DEBUG_ALL, "Sending: " + hex);

	// NOTE: this "1" indicates that the vehicle will reply with a 1-line response
	std::string send_str = hex + "\r";
	if (sio -> write(send_str.c_str(), send_str.size()) == -1)
		error_exit("Problem writing %s to ELM327", send_str.c_str());

	std::string in;

	byte_array *line = NULL;
	unsigned int offset = 0;

	bool first = true, can_response = false;
	for(;;)
	{
		in = trim(recv());

		t -> emit(LL_DEBUG_ALL, " received: |" + in + "|");

		if (in == ">")
			break;

		if (in == "")
			continue;

		if (in.substr(0, 6) == "ELM327") // letf-over of the "ATZ" command
			continue;

		if (in == "STOPPED" || // device was interrupted, retry
			in == "CAN ERROR")
		{
			if (in == "STOPPED")
				t -> emit(LL_ERROR, " ELM327 was interrupted");
			else
			{
				t -> emit(LL_ERROR, " CAN error");

				usleep(100000); // sleep 100ms so that we're sure that the bus is idle

				std::string reset1 = "AT FE\r";
				if (sio -> write(reset1.c_str(), reset1.size()) == -1)
					error_exit("Problem writing %s to ELM327", reset1.c_str());

				usleep(25000); // sleep 25ms

				std::string reset2 = "AT Z\r";
				if (sio -> write(reset2.c_str(), reset2.size()) == -1)
					error_exit("Problem writing %s to ELM327", reset2.c_str());
			}

			n_sr_fail++;

			usleep(100000); // sleep 100ms so that we're sure that the bus is idle

			sio -> flush(); // reply of AT FE for example

			return;
		}

		if (in == "SEARCHING..." || in.substr(0, 8) == "BUS INIT") // first setup of can-bus connection
		{
			t -> emit(LL_INFO, " SEARCHING/BUS INIT");

			usleep(100000); // sleep 100ms so that we're sure that the bus is idle

			sio -> flush();

			continue;
		}

		if (in == "UNABLE TO CONNECT")
			error_exit("Is the car switched on?");

		if (in == "NO DATA")	// retrieving data from a not supported pid
		{
			n_sr_fail++;

			return;
		}

		if (in == "?")	// command was not understood by elm327 device
		{
			n_sr_fail++;

			return;
		}

                if (in == "51")
                {
                        t -> emit(LL_INFO, " Ignoring strange unexpected 0x51 byte");

                        n_sr_fail++;

                        usleep(100000); // sleep 100ms
                        sio -> flush();

                        return;
                }

		std::vector<std::string> hex_array = split_string(in, " ");
		if (first && hex_array.size() == 1 && hex_array.at(0).size() == 3) // CAN response
		{
			can_response = true;
		}
		else
		{
			// CAN: first column is index number followed by :
			unsigned int start_index = can_response ? 1 : 0;

			if (!line)
				line = new byte_array();

			for(unsigned int index=start_index; index<hex_array.size(); index++)
			{
				int value = hex_to_int(hex_array.at(index));
				if (value == -1)
					error_exit("Got non-hex value");

				line -> set(offset++, (char)value);
			}

			if (!can_response)
			{
				results -> push_back(line);
				line = NULL;
				offset = 0;
			}
		}

		first = false;
	}

	if (can_response)
		results -> push_back(line);

	n_sr_ok++;
}

std::string IO_ELM327::get_source_type()
{
	return "ELM327 CAN-bus adapter (" + dev_info + ")";
}

bool IO_ELM327::has_sensor(std::string s)
{
	if (s == "battery_voltage")
		return true;

	return false;
}

bool IO_ELM327::get_sensor(std::string s, double *value)
{
	if (s == "battery_voltage")
	{
		sio -> flush();

		std::string cmd = "AT RV";
		t -> emit(LL_DEBUG_ALL, "send: " + cmd);
		if (sio -> write((cmd + "\r").c_str(), cmd.size() + 1) == -1)
			error_exit("Problem writing %s to ELM327", cmd.c_str());

		std::string result = recv();
		t -> emit(LL_DEBUG_ALL, "recv: " + result);

		unsigned int result_len = result.size();
		if (result.substr(result_len - 1) == "V")
		{
			*value = atof(result.substr(0, result_len - 1).c_str());

			wait_for_prompt();

			return true;
		}

		t -> emit(LL_DEBUG_SOME, format("ELM327 returned %s for %s which is not expected", result.c_str(), cmd.c_str()));

		wait_for_prompt();

		return false;
	}

	t -> emit(LL_ERROR, "*** UNSUPPORTED SENSOR ***");

	return false;
}

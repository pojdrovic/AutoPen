// SVN: $Revision: 463 $
#include <math.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "byte_array.h"
#include "terminal.h"
#include "utils.h"
#include "IO_Device.h"
#include "obd2-emulation.h"
#include "IO_Test.h"

IO_Test::IO_Test(terminal *t_in, bool full_speed_in) : IO_Device(t_in, full_speed_in)
{
}

IO_Test::~IO_Test()
{
}

void IO_Test::send_recv(byte_array *command, std::vector<byte_array *> *results, int result_count_limit)
{
	slowdown(); // slow down to once every 10ms a message

	emulate_obd2(t, command, results);

	if (result_count_limit >= 1 && result_count_limit < 10)
	{
		while(results -> size() > result_count_limit)
		{
			delete results -> at(1);
			results -> erase(results -> begin() + 1);
		}
	}
	else if (result_count_limit != -1)
	{
		t -> emit(LL_ERROR, format("ELM327: result limit %d not understood (must be either -1 or 1 <= 1 < 10), ignoring", result_count_limit));
	}
}

std::string IO_Test::get_source_type()
{
	return "fake vehicle";
}

bool IO_Test::has_sensor(std::string s)
{
	if (s == "battery_voltage")
		return true;

	return false;
}

bool IO_Test::get_sensor(std::string s, double *value)
{
	if (s == "battery_voltage")
	{
		*value = 12.0 + sin(get_ts() * PI / 180.0);
		return true;
	}

	t -> emit(LL_ERROR, "*** UNSUPPORTED SENSOR ***");

	return false;
}

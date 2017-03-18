#include <map>
#include <string>
#include <vector>

#include "../error.h"
#include "../terminal.h"
#include "../byte_array.h"
#include "../IO_Device.h"
#include "sensor.h"
#include "io_sensor.h"
#include "../utils.h"
#include "../OBD2.h"
#include "../main.h"

sensor::sensor() : error_count(0)
{
}

sensor::~sensor()
{
}

bool sensor::get_value(terminal *t, OBD2 *io, std::string *value)
{
	double dummy = -1.0;

	if (!get_value(t, io, &dummy))
		return false;

	value -> assign(format(get_screen_formatting().c_str(), dummy));

	return true;
}

bool sensor::get_value(terminal *t, OBD2 *io, double *value)
{
	if (error_count > SENSOR_MAX_ERROR_BEFORE_IGNORE)
		return false;

	int mode = get_mode();

	if (mode != 0x01)
		error_exit("Expecting PIDs with mode 0x01 (not %02x)", mode);

	int processor = get_processor();

	bool rc = false;
	switch(processor)
	{
		case ppid_16b:
			rc = io -> get_pid_16b(get_pid(), value);
			break;
		case ppid_16b_div_1000:
			rc = io -> get_pid_16b_div_1000(get_pid(), value);
			break;
		case ppid_16b_double:
			rc = io -> get_pid_16b_double(get_pid(), value);
			break;
		case ppid_16b_div100:
			rc = io -> get_pid_16b_div100(get_pid(), value);
			break;
		case ppid_16b_div10_sub40:
			rc = io -> get_pid_16b_div10_sub40(get_pid(), value);
			break;
		case ppid_16b_div2:
			rc = io -> get_pid_16b_div2(get_pid(), value);
			break;
		case ppid_16b_div32768:
			rc = io -> get_pid_16b_div32768(get_pid(), value);
			break;
		case ppid_16b_div4:
			rc = io -> get_pid_16b_div4(get_pid(), value);
			break;
		case ppid_16b_percent_25700:
			rc = io -> get_pid_16b_percent_25700(get_pid(), value);
			break;
		case ppid_16b_mul0_05:
			rc = io -> get_pid_16b_mul0_05(get_pid(), value);
			break;
		case ppid_16b_mul0_079:
			rc = io -> get_pid_16b_mul0_079(get_pid(), value);
			break;
		case ppid_16b_mul10:
			rc = io -> get_pid_16b_mul10(get_pid(), value);
			break;
		case ppid_16b_sub26880_div128:
			rc = io -> get_pid_16b_sub26880_div128(get_pid(), value);
			break;
		case ppid_3B_div200:
			rc = io -> get_pid_3B_div200(get_pid(), value);
			break;
		case ppid_8b_a:
			rc = io -> get_pid_8b_a(get_pid(), value);
			break;
		case ppid_8b_b:
			rc = io -> get_pid_8b_b(get_pid(), value);
			break;
		case ppid_8b_c:
			rc = io -> get_pid_8b_c(get_pid(), value);
			break;
		case ppid_8b_mul10_d:
			rc = io -> get_pid_8b_mul10_d(get_pid(), value);
			break;
		case ppid_ab_div32768:
			rc = io -> get_pid_ab_div32768(get_pid(), value);
			break;
		case ppid_cd_div8192:
			rc = io -> get_pid_cd_div8192(get_pid(), value);
			break;
		case ppid_div2_sub64:
			rc = io -> get_pid_div2_sub64(get_pid(), value);
			break;
		case ppid_mul3:
			rc = io -> get_pid_mul3(get_pid(), value);
			break;
		case ppid_percent:
			rc = io -> get_pid_percent(get_pid(), value);
			break;
		case ppid_percent128:
			rc = io -> get_pid_percent128(get_pid(), value);
			break;
		case ppid_pid_double:
			rc = io -> get_pid_double(get_pid(), value);
			break;
		case ppid_sub125:
			rc = io -> get_pid_sub125(get_pid(), value);
			break;
		case ppid_sub40:
			rc = io -> get_pid_sub40(get_pid(), value);
			break;
		case ppid_string:
			rc = false; // use the other get_value
			break;
		case io_get:
			rc = io -> get_io_sensor(get_symbol(), value);
			break;
		default:
			error_exit("Unknown PID processor %d", processor);
	}

	if (rc)
		error_count = 0;
	else
	{
		error_count++;

		if (error_count > SENSOR_MAX_ERROR_BEFORE_IGNORE)
			t -> emit(LL_ERROR, format("Sensor %s (%02x%02x) failed %d times, will be ignored from now on", get_symbol().c_str(), get_mode(), get_pid(), error_count));
	}

	return rc;
}

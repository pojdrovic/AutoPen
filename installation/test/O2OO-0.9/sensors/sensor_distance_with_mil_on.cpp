#include <map>
#include <string>
#include <vector>

#include "../error.h"
#include "../terminal.h"
#include "../byte_array.h"
#include "../IO_Device.h"
#include "sensor.h"
#include "sensor_distance_with_mil_on.h"
#include "io_sensor.h"
#include "../utils.h"
#include "../OBD2.h"

bool sensor_distance_with_mil_on::get_value(OBD2 *o, std::string *value)
{
	byte_array *result = o -> get_pid(get_pid());
	if (!result)
		return false;

	if (result -> size() != 4)
	{
		delete result;
		return false;
	}

	value -> assign(format("%d", (result -> get_uchar(2) << 8) + result -> get_uchar(3)) + get_unit());

	delete result;

	return true;
}

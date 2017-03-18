#include <map>
#include <string>
#include <vector>

#include "../error.h"
#include "../terminal.h"
#include "../byte_array.h"
#include "../IO_Device.h"
#include "sensor.h"
#include "sensor_warmups_after_code_cleared.h"
#include "io_sensor.h"
#include "../utils.h"
#include "../OBD2.h"

bool sensor_warmups_after_code_cleared::get_value(OBD2 *o, std::string *value)
{
	byte_array *result = o -> get_pid(get_pid());
	if (!result)
		return false;

	if (result -> size() != 3)
	{
		delete result;
		return false;
	}

	value -> assign(format("%d", result -> get_uchar(2)));

	delete result;

	return true;
}

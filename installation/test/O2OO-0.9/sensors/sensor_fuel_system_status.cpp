#include <map>
#include <string>
#include <vector>

#include "../error.h"
#include "../terminal.h"
#include "../byte_array.h"
#include "../IO_Device.h"
#include "sensor.h"
#include "sensor_fuel_system_status.h"
#include "io_sensor.h"
#include "../utils.h"
#include "../OBD2.h"

bool sensor_fuel_system_status::get_value(OBD2 *o, std::string *value)
{
	byte_array *result = o -> get_pid(get_pid());
	if (!result)
		return false;

	if (result -> size() != 4)
	{
		delete result;
		return false;
	}

	std::string temp;
	if (result -> get_uchar(2) & 1)
		temp += "open loop due to insufficient engine temperature";

	if (result -> get_uchar(2) & 2)
	{
		if (temp != "")
			temp += "\n";
		temp += "closed loop, using oxygen sensor feedback to determine fuel mix";
	}

	if (result -> get_uchar(2) & 4)
	{
		if (temp != "")
			temp += "\n";
		temp += "open loop due to engine load OR fuel cut due to deceleration";
	}

	if (result -> get_uchar(2) & 8)
	{
		if (temp != "")
			temp += "\n";
		temp += "open loop due to system failure";
	}

	if (result -> get_uchar(2) & 16)
	{
		if (temp != "")
			temp += "\n";
		temp += "closed loop, using at least one oxygen sensor but\nthere is a fault in the feedback system";
	}

	value -> assign(temp);

	delete result;

	return true;
}

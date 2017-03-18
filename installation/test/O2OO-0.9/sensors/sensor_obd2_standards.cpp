#include <map>
#include <string>
#include <vector>

#include "../error.h"
#include "../terminal.h"
#include "../byte_array.h"
#include "../IO_Device.h"
#include "sensor.h"
#include "sensor_obd2_standards.h"
#include "io_sensor.h"
#include "../utils.h"
#include "../OBD2.h"

bool sensor_obd2_standards::get_value(OBD2 *o, std::string *value)
{
	byte_array *result = o -> get_pid(get_pid());
	if (!result)
		return false;

	if (result -> size() != 3)
	{
		delete result;
		return false;
	}

	std::string temp = "?";
	switch(result -> get_uchar(2))
	{
		case 0x01:
			temp = "OBD-II as defined by the CARB"; break;
		case 0x02:
			temp = "OBD as defined by the EPA"; break;
		case 0x03:
			temp = "OBD and OBD-II"; break;
		case 0x04:
			temp = "OBD-I"; break;
		case 0x05:
			temp = "Not meant to comply with any OBD standard"; break;
		case 0x06:
			temp = "EOBD (Europe)"; break;
		case 0x07:
			temp = "EOBD and OBD-II"; break;
		case 0x08:
			temp = "EOBD and OBD"; break;
		case 0x09:
			temp = "EOBD, OBD and OBD II"; break;
		case 0x0a:
			temp = "JOBD (Japan)"; break;
		case 0x0b:
			temp = "JOBD and OBD II"; break;
		case 0x0c:
			temp = "JOBD and EOBD"; break;
		case 0x0d:
			temp = "JOBD, EOBD, and OBD II"; break;
	}

	value -> assign(temp);

	delete result;

	return true;
}

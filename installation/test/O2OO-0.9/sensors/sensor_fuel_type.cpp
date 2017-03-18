#include <map>
#include <string>
#include <vector>

#include "../error.h"
#include "../terminal.h"
#include "../byte_array.h"
#include "../IO_Device.h"
#include "sensor.h"
#include "sensor_fuel_type.h"
#include "io_sensor.h"
#include "../utils.h"
#include "../OBD2.h"

bool sensor_fuel_type::get_value(OBD2 *o, std::string *value)
{
	byte_array *result = o -> get_pid(get_pid());
	if (!result)
		return false;

	if (result -> size() != 3)
	{
		delete result;
		return false;
	}

	int fuel_type = result -> get_uchar(2);
	switch(fuel_type)
	{
		case 0x01:
			value -> assign("Gasoline");
			break;
		case 0x02:
			value -> assign("Methanol");
			break;
		case 0x03:
			value -> assign("Ethanol");
			break;
		case 0x04:
			value -> assign("Diesel");
			break;
		case 0x05:
			value -> assign("LPG");
			break;
		case 0x06:
			value -> assign("CNG");
			break;
		case 0x07:
			value -> assign("Propane");
			break;
		case 0x08:
			value -> assign("Electric");
			break;
		case 0x09:
			value -> assign("Bifuel running Gasoline");
			break;
		case 0x0A:
			value -> assign("Bifuel running Methanol");
			break;
		case 0x0B:
			value -> assign("Bifuel running Ethanol");
			break;
		case 0x0C:
			value -> assign("Bifuel running LPG");
			break;
		case 0x0D:
			value -> assign("Bifuel running CNG");
			break;
		case 0x0E:
			value -> assign("Bifuel running Prop");
			break;
		case 0x0F:
			value -> assign("Bifuel running Electricity");
			break;
		case 0x10:
			value -> assign("Bifuel mixed gas/electric");
			break;
		case 0x11:
			value -> assign("Hybrid gasoline");
			break;
		case 0x12:
			value -> assign("Hybrid Ethanol");
			break;
		case 0x13:
			value -> assign("Hybrid Diesel");
			break;
		case 0x14:
			value -> assign("Hybrid Electric");
			break;
		case 0x15:
			value -> assign("Hybrid Mixed fuel");
			break;
		case 0x16:
			value -> assign("Hybrid Regenerative");
			break;
		default:
			value -> assign(format("%02x?", fuel_type));
			break;
	}

	delete result;

	return true;
}

// SVN: $Revision: 464 $
#include <iostream>
#include <map>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

#include "error.h"
#include "byte_array.h"
#include "utils.h"
#include "terminal.h"
#include "IO_Device.h"
#include "sensors/sensor.h"
#include "sensors/io_sensor.h"
#include "OBD2.h"
#include "sensors/sensor_absolute_load_value.h"
#include "sensors/sensor_accelerator_pedal_position_d.h"
#include "sensors/sensor_accelerator_pedal_position_e.h"
#include "sensors/sensor_accelerator_pedal_position_f.h"
#include "sensors/sensor_actual_engine_torque.h"
#include "sensors/sensor_ambient_air_temperature.h"
#include "sensors/sensor_barometric_pressure.h"
#include "sensors/sensor_catalyst_temperature_bank_1_sensor_1.h"
#include "sensors/sensor_catalyst_temperature_bank_1_sensor_2.h"
#include "sensors/sensor_catalyst_temperature_bank_2_sensor_1.h"
#include "sensors/sensor_catalyst_temperature_bank_2_sensor_2.h"
#include "sensors/sensor_commanded_egr.h"
#include "sensors/sensor_commanded_evaporative_purge.h"
#include "sensors/sensor_commanded_throttle_actuator.h"
#include "sensors/sensor_command_equivalence_ratio.h"
#include "sensors/sensor_control_module_voltage.h"
#include "sensors/sensor_distance_traveled_since_codes_cleared.h"
#include "sensors/sensor_driver_demand_torque.h"
#include "sensors/sensor_egr_error.h"
#include "sensors/sensor_engine_coolant_temperature.h"
#include "sensors/sensor_engine_fuel_rate.h"
#include "sensors/sensor_engine_load.h"
#include "sensors/sensor_engine_rpm.h"
#include "sensors/sensor_engine_oil_temperature.h"
#include "sensors/sensor_engine_reference_torque.h"
#include "sensors/sensor_ethanol_fuel.h"
#include "sensors/sensor_evap_system_vapor_pressure.h"
#include "sensors/sensor_fuel_injection_timing.h"
#include "sensors/sensor_fuel_level_input.h"
#include "sensors/sensor_fuel_pressure.h"
#include "sensors/sensor_fuel_rail_pressure_direct_inject.h"
#include "sensors/sensor_fuel_rail_pressure_manifold_vacuum.h"
#include "sensors/sensor_fuel_type.h"
#include "sensors/sensor_intake_air_temperature.h"
#include "sensors/sensor_intake_manifold_abs_pressure.h"
#include "sensors/sensor_long_term_fuel_bank1.h"
#include "sensors/sensor_long_term_fuel_bank2.h"
#include "sensors/sensor_maf_air_flow_rate.h"
#include "sensors/sensor_maximum_equivalance_ratio.h"
#include "sensors/sensor_maximum_intake_manifold_abs_pressure.h"
#include "sensors/sensor_maximum_oxygen_sensor_current.h"
#include "sensors/sensor_maximum_oxygen_sensor_voltage.h"
#include "sensors/sensor_o2s1_wr_lambda_voltage.h"
#include "sensors/sensor_o2s2_wr_lambda_voltage.h"
#include "sensors/sensor_o2s3_wr_lambda_voltage.h"
#include "sensors/sensor_o2s4_wr_lambda_voltage.h"
#include "sensors/sensor_o2s5_wr_lambda_voltage.h"
#include "sensors/sensor_o2s6_wr_lambda_voltage.h"
#include "sensors/sensor_o2s7_wr_lambda_voltage.h"
#include "sensors/sensor_o2s8_wr_lambda_voltage.h"
#include "sensors/sensor_o2s1_wr_lambda_equivalence_ratio.h"
#include "sensors/sensor_o2s2_wr_lambda_equivalence_ratio.h"
#include "sensors/sensor_o2s3_wr_lambda_equivalence_ratio.h"
#include "sensors/sensor_o2s4_wr_lambda_equivalence_ratio.h"
#include "sensors/sensor_o2s5_wr_lambda_equivalence_ratio.h"
#include "sensors/sensor_o2s6_wr_lambda_equivalence_ratio.h"
#include "sensors/sensor_o2s7_wr_lambda_equivalence_ratio.h"
#include "sensors/sensor_o2s8_wr_lambda_equivalence_ratio.h"
#include "sensors/sensor_oxygen_sensor_bank1_sensor1.h"
#include "sensors/sensor_oxygen_sensor_bank1_sensor2.h"
#include "sensors/sensor_oxygen_sensor_bank1_sensor3.h"
#include "sensors/sensor_oxygen_sensor_bank1_sensor4.h"
#include "sensors/sensor_oxygen_sensor_bank2_sensor1.h"
#include "sensors/sensor_oxygen_sensor_bank2_sensor2.h"
#include "sensors/sensor_oxygen_sensor_bank2_sensor3.h"
#include "sensors/sensor_oxygen_sensor_bank2_sensor4.h"
#include "sensors/sensor_relative_throttle_position.h"
#include "sensors/sensor_short_term_fuel_bank1.h"
#include "sensors/sensor_short_term_fuel_bank2.h"
#include "sensors/sensor_throttle.h"
#include "sensors/sensor_timing_advance.h"
#include "sensors/sensor_vehicle_speed.h"
//
#include "sensors/sensor_distance_with_mil_on.h"
#include "sensors/sensor_fuel_system_status.h"
#include "sensors/sensor_obd2_standards.h"
#include "sensors/sensor_warmups_after_code_cleared.h"
//
#include "sensors/io_sensor_battery_voltage.h"

OBD2::OBD2(IO_Device *io_in, terminal *t_in) : io(io_in), t(t_in)
{
	n_ok = n_fail = 0;
	avg_ts = 0;

	memset(is_supported, 0x00, sizeof is_supported);

	add_sensor(new sensor_absolute_load_value());
	add_sensor(new sensor_accelerator_pedal_position_d());
	add_sensor(new sensor_accelerator_pedal_position_e());
	add_sensor(new sensor_accelerator_pedal_position_f());
	add_sensor(new sensor_actual_engine_torque());
	add_sensor(new sensor_ambient_air_temperature());
	add_sensor(new sensor_barometric_pressure());
	add_sensor(new sensor_catalyst_temperature_bank_1_sensor_1());
	add_sensor(new sensor_catalyst_temperature_bank_1_sensor_2());
	add_sensor(new sensor_catalyst_temperature_bank_2_sensor_1());
	add_sensor(new sensor_catalyst_temperature_bank_2_sensor_2());
	add_sensor(new sensor_command_equivalence_ratio());
	add_sensor(new sensor_commanded_egr());
	add_sensor(new sensor_commanded_evaporative_purge());
	add_sensor(new sensor_commanded_throttle_actuator());
	add_sensor(new sensor_control_module_voltage());
	add_sensor(new sensor_distance_traveled_since_codes_cleared());
	add_sensor(new sensor_driver_demand_torque());
	add_sensor(new sensor_egr_error());
	add_sensor(new sensor_engine_coolant_temperature());
	add_sensor(new sensor_engine_reference_torque());
	add_sensor(new sensor_engine_fuel_rate());
	add_sensor(new sensor_engine_load());
	add_sensor(new sensor_engine_rpm());
	add_sensor(new sensor_engine_oil_temperature());
	add_sensor(new sensor_ethanol_fuel());
	add_sensor(new sensor_evap_system_vapor_pressure());
	add_sensor(new sensor_fuel_injection_timing());
	add_sensor(new sensor_fuel_level_input());
	add_sensor(new sensor_fuel_pressure());
	add_sensor(new sensor_fuel_rail_pressure_direct_inject());
	add_sensor(new sensor_fuel_rail_pressure_manifold_vacuum());
	add_sensor(new sensor_intake_air_temperature());
	add_sensor(new sensor_intake_manifold_abs_pressure());
	add_sensor(new sensor_long_term_fuel_bank1());
	add_sensor(new sensor_long_term_fuel_bank2());
	add_sensor(new sensor_maf_air_flow_rate());
	add_sensor(new sensor_maximum_equivalance_ratio());
	add_sensor(new sensor_maximum_intake_manifold_abs_pressure());
	add_sensor(new sensor_maximum_oxygen_sensor_current());
	add_sensor(new sensor_maximum_oxygen_sensor_voltage());
	add_sensor(new sensor_o2s1_wr_lambda_voltage());
	add_sensor(new sensor_o2s2_wr_lambda_voltage());
	add_sensor(new sensor_o2s3_wr_lambda_voltage());
	add_sensor(new sensor_o2s4_wr_lambda_voltage());
	add_sensor(new sensor_o2s5_wr_lambda_voltage());
	add_sensor(new sensor_o2s6_wr_lambda_voltage());
	add_sensor(new sensor_o2s7_wr_lambda_voltage());
	add_sensor(new sensor_o2s8_wr_lambda_voltage());
	add_sensor(new sensor_o2s1_wr_lambda_equivalence_ratio());
	add_sensor(new sensor_o2s2_wr_lambda_equivalence_ratio());
	add_sensor(new sensor_o2s3_wr_lambda_equivalence_ratio());
	add_sensor(new sensor_o2s4_wr_lambda_equivalence_ratio());
	add_sensor(new sensor_o2s5_wr_lambda_equivalence_ratio());
	add_sensor(new sensor_o2s6_wr_lambda_equivalence_ratio());
	add_sensor(new sensor_o2s7_wr_lambda_equivalence_ratio());
	add_sensor(new sensor_o2s8_wr_lambda_equivalence_ratio());
	add_sensor(new sensor_oxygen_sensor_bank1_sensor1());
	add_sensor(new sensor_oxygen_sensor_bank1_sensor2());
	add_sensor(new sensor_oxygen_sensor_bank1_sensor3());
	add_sensor(new sensor_oxygen_sensor_bank1_sensor4());
	add_sensor(new sensor_oxygen_sensor_bank2_sensor1());
	add_sensor(new sensor_oxygen_sensor_bank2_sensor2());
	add_sensor(new sensor_oxygen_sensor_bank2_sensor3());
	add_sensor(new sensor_oxygen_sensor_bank2_sensor4());
	add_sensor(new sensor_relative_throttle_position());
	add_sensor(new sensor_short_term_fuel_bank1());
	add_sensor(new sensor_short_term_fuel_bank2());
	add_sensor(new sensor_throttle());
	add_sensor(new sensor_timing_advance());
	add_sensor(new sensor_vehicle_speed());

	add_sensor(new sensor_distance_with_mil_on());
	add_sensor(new sensor_fuel_system_status());
	add_sensor(new sensor_obd2_standards());
	add_sensor(new sensor_fuel_type());
	add_sensor(new sensor_warmups_after_code_cleared());

	add_io_sensor(new io_sensor_battery_voltage());

	determine_supported_mode_0x01_pids();
}

void OBD2::add_io_sensor(io_sensor *s)
{
	std::string symbol = s -> get_symbol();
	if (io -> has_sensor(symbol))
	{
		t -> emit(LL_DEBUG_SOME, "OBD2: adding I/O sensor " + symbol);
		add_sensor(s);

		symbol_to_object_detected.insert(std::pair<std::string, sensor *>(symbol, s));
	}
	else
	{
		t -> emit(LL_DEBUG_SOME, "OBD2: I/O device does not have sensor " + s -> get_symbol());
		delete s;
	}
}

void OBD2::add_sensor(sensor *s)
{
	std::string symbol = s -> get_symbol();

	symbol_to_object_available.insert(std::pair<std::string, sensor *>(symbol, s));

	mode_pid_to_object_available.insert(std::pair<int, sensor *>((s -> get_mode() << 8) | s -> get_pid(), s));
}

void OBD2::determine_supported_mode_0x01_pids()
{
	t -> emit(LL_DEBUG_SOME, "OBD2: determing supported pids...");

	is_supported[0] = true;

	std::vector<byte_array *> replies;

	for(int index=0; index<6; index++)
	{
		int offset = index * 0x20;

		if (!is_supported[offset])
			break;

		t -> emit(LL_DEBUG_ALL, format("OBD2: requesting bitmap for %02x (%d)", offset, offset));

		byte_array command;
		command.set(0, 0x01);
		command.set(1, offset);

		io -> send_recv(&command, &replies);

		if (replies.size() == 0)
		{
			t -> emit(LL_ERROR, "No reply from device");
			continue;
		}

		bool valid = false;
		unsigned char bits[4] = { 0 };
		for(unsigned int repindex=0; repindex<replies.size(); repindex++)
		{
			byte_array *reply = replies.at(repindex);

			if (reply -> size() != 6)
			{
				n_fail++;
				io -> flush();
				t -> emit(LL_ERROR, format("> Unexpected reply size %d (expected 2 header and 4 data bytes)", reply -> size()));
				continue;
			}

			// FIXME verify if reply is for current offset? if fail, retry?
			if (reply -> get_uchar(1) != offset)
			{
				t -> emit(LL_ERROR, format("> Bitmap for unexpected offset %02x (expected: %02x)", reply -> get_uchar(1), offset));
				continue;
			}

			bits[0] |= reply -> get_uchar(2);
			bits[1] |= reply -> get_uchar(3);
			bits[2] |= reply -> get_uchar(4);
			bits[3] |= reply -> get_uchar(5);

			valid = true;
		}

		if (valid)
		{
			byte_array bits_ba(bits, sizeof bits);

			std::string debug;
			for(int bit=0; bit<32; bit++)
			{
				// bit offset 16: byte 0=0x41 (0x40 + command (0x01), byte for the requested pid)
				is_supported[offset + 1 + bit] = bits_ba.get_bit_le(bit);

				if (bits_ba.get_bit_le(bit))
					debug += "1";
				else
					debug += "0";
			}

			t -> emit(LL_DEBUG_SOME, " pid bitmap: " + debug);
		}

		for(unsigned int delindex=0; delindex<replies.size(); delindex++)
			delete replies.at(delindex);
		replies.clear();
	}

	for(unsigned int delindex=0; delindex<replies.size(); delindex++)
		delete replies.at(delindex);

	std::map<std::string, sensor *>::iterator it = symbol_to_object_available.begin();
	for(;it != symbol_to_object_available.end(); it++)
	{
		int offset = it -> second -> get_pid();

		if (!is_supported[offset])
			continue;

		symbol_to_object_detected.insert(std::pair<std::string, sensor *>(it -> second -> get_symbol(), it -> second));
	}

        for(int offset=0; offset<256; offset++)
        {
                if (!is_supported[offset])
                        continue;

		if (offset % 0x20 == 0)
			continue;

                std::map<int, sensor *>::iterator it = mode_pid_to_object_available.find((0x01 << 8) | offset);
                if (it == mode_pid_to_object_available.end())
                {
                        t -> emit(LL_ERROR, format("No processor for PID 0x01 0x%02x", offset));
                        n_fail++;
                }
        }
}

bool OBD2::get_is_supported(int mode, int pid)
{
	std::map<std::string, sensor *>::iterator it = symbol_to_object_detected.begin();

	for(; it != symbol_to_object_detected.end(); it++)
	{
		if (it -> second -> get_mode() == mode && it -> second -> get_pid() == pid)
			return true;
	}

	return false;
}

void OBD2::list_supported_pids()
{
	std::map<std::string, sensor *>::iterator it = symbol_to_object_detected.begin();

	for(; it != symbol_to_object_detected.end(); it++)
	{
		std::string symbol = it -> first;
		int PID = (it -> second -> get_mode() << 8) | it -> second -> get_pid();

		t -> emit(LL_INFO, format("%04x\t", PID) + symbol);
	}
}

bool OBD2::ignore_sensor(std::string name)
{
	std::map<std::string, sensor *>::iterator it = symbol_to_object_detected.find(name);

	if (it != symbol_to_object_detected.end())
	{
		int pid = it -> second -> get_pid();
		int nr = (it -> second -> get_mode() << 8) | pid;
		symbol_to_object_detected.erase(it);

		std::map<int, sensor *>::iterator it2 = mode_pid_to_object_available.find(nr);
		mode_pid_to_object_available.erase(it2);

		is_supported[pid] = false;

		return true;
	}

	t -> emit(LL_ERROR, "OB2 ignore sensor: " + name + " is not known");

	return false;
}

byte_array * OBD2::get_pid(int pid)
{
	byte_array command;
	std::vector<byte_array *> replies;

	double start_ts = get_ts();
	for(;;)
	{
		command.set(0, 0x01);
		command.set(1, pid);
		io -> send_recv(&command, &replies);

		if (replies.size() == 0)
		{
			t -> emit(LL_ERROR, "No reply from device");
			break;
		}

		byte_array *reply = replies.at(0);

		if (reply -> size() < 2)
		{
			t -> emit(LL_ERROR, format("Truncated reply (%d bytes in size)", reply -> size()));
			break;
		}

		// FIXME clear buffer & retry
		if (reply -> get_uchar(0) != 0x41) // NOTE: change if also using other modes
		{
			t -> emit(LL_ERROR, format("Unexpected reply 0x%02x (expected 0x41)", reply -> get_char(0)));
			break;
		}

		if (reply -> get_uchar(1) != pid)
		{
			t -> emit(LL_ERROR, format("Reply for pid (0x%02x) different from the one requested (0x%02x)", reply -> get_uchar(1), pid));
			break;
		}

		n_ok++;
		double end_ts = get_ts();
		avg_ts += end_ts - start_ts;

		for(unsigned int delindex=1; delindex<replies.size(); delindex++)
			delete replies.at(delindex);

		return reply;
	}

	io -> flush();

	for(unsigned int delindex=0; delindex<replies.size(); delindex++)
		delete replies.at(delindex);

	n_fail++;

	return NULL;;
}

bool OBD2::get_pid_8b(int pid, int *ret, int offset)
{
	byte_array *data = NULL;

	for(;;)
	{
		if (!is_supported[pid])
		{
			t -> emit(LL_ERROR, format("Requesting unsupported pid %02x", pid));
			break;
		}

		data = get_pid(pid);

		if (!data || data -> size() == 0)
			break;

		if (data -> size() < 3 + offset)
		{
			t -> emit(LL_ERROR, format("Unexpected reply size (%d bytes, expected %d)", data -> size(), 3 + offset));
			break;
		}

		*ret = data -> get_uchar(2 + offset);

		delete data;

		return true;
	}

	delete data;

	n_fail++;

	io -> flush();

	return false;
}

bool OBD2::get_pid_16b(int pid, int *ret)
{
	byte_array *data = NULL;

	for(;;)
	{
		if (!is_supported[pid])
		{
			t -> emit(LL_ERROR, format("Requesting unsupported pid %02x", pid));
			break;
		}

		data = get_pid(pid);

		if (!data || data -> size() == 0)
			break;

		if (data -> size() != 4)
		{
			t -> emit(LL_ERROR, format("Unexpected reply size (%d bytes, expected 4)", data -> size()));
			break;
		}

		*ret = (data -> get_uchar(2) << 8) + data -> get_uchar(3);

		delete data;

		return true;
	}

	n_fail++;

	io -> flush();

	delete data;

	return false;
}

bool OBD2::get_pid_8b_a(int pid, double *ret)
{
	int dummy = -1;

	if (get_pid_8b(pid, &dummy, 0))
	{
		*ret = double(dummy);

		return true;
	}

	return false;
}

bool OBD2::get_pid_8b_b(int pid, double *ret)
{
	int dummy = -1;

	if (get_pid_8b(pid, &dummy, 1))
	{
		*ret = double(dummy);

		return true;
	}

	return false;
}

bool OBD2::get_pid_8b_c(int pid, double *ret)
{
	int dummy = -1;

	if (get_pid_8b(pid, &dummy, 2))
	{
		*ret = double(dummy);

		return true;
	}

	return false;
}

bool OBD2::get_pid_8b_mul10_d(int pid, double *ret)
{
	int dummy = -1;

	if (get_pid_8b(pid, &dummy, 3))
	{
		*ret = double(dummy) * 10.0;

		return true;
	}

	return false;
}

bool OBD2::get_pid_16b(int pid, double *ret)
{
	int dummy = -1;

	if (get_pid_16b(pid, &dummy))
	{
		*ret = double(dummy);

		return true;
	}

	return false;
}

bool OBD2::get_pid_16b_signed(int pid, int *ret)
{
	byte_array *data = NULL;

	for(;;)
	{
		if (!is_supported[pid])
		{
			t -> emit(LL_ERROR, format("Requesting unsupported pid %02x", pid));
			break;
		}

		data = get_pid(pid);

		if (!data || data -> size() == 0)
			break;

		if (data -> size() != 4)
		{
			t -> emit(LL_ERROR, format("Unexpected reply size (%d bytes, expected 4)", data -> size()));
			break;
		}

		*ret = (data -> get_char(2) << 8) + data -> get_uchar(3);

		delete data;

		return true;
	}

	n_fail++;

	io -> flush();

	delete data;

	return false;
}

bool OBD2::get_pid_percent(int pid, double *ret)
{
	int dummy;

	if (!get_pid_8b(pid, &dummy))
		return false;

	*ret = double(dummy) * 100.0 / 255.0;

	return true;
}

bool OBD2::get_pid_16b_percent_25700(int pid, double *ret)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*ret = double(dummy) * 100.0 / 255.0;

	return true;
}

bool OBD2::get_pid_sub125(int pid, double *ret)
{
	int dummy;

	if (!get_pid_8b(pid, &dummy))
		return false;

	*ret = double(dummy) - 125.0;

	return true;
}

bool OBD2::get_pid_percent128(int pid, double *ret)
{
	int dummy;

	if (!get_pid_8b(pid, &dummy))
		return false;

	*ret = double(dummy - 128) * 100.0 / 128.0;

	return true;
}

bool OBD2::get_pid_mul3(int pid, double *ret)
{
	int dummy;

	if (!get_pid_8b(pid, &dummy))
		return false;

	*ret = double(dummy * 3);

	return true;
}

bool OBD2::get_pid_16b_div4(int pid, double *ret)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*ret = double(dummy) / 4.0;

	return true;
}

bool OBD2::get_pid_16b_div100(int pid, double *ret)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*ret = double(dummy) / 100.0;

	return true;
}

bool OBD2::get_pid_div2_sub64(int pid, double *ret)
{
	int dummy;

	if (!get_pid_8b(pid, &dummy))
		return false;

	*ret = double(dummy) / 2.0 - 64.0;

	return true;
}

bool OBD2::get_pid_16b_div2(int pid, double *ret)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*ret = double(dummy) / 2.0;

	return true;
}

bool OBD2::get_pid_16b_div32768(int pid, double *ret)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*ret = double(dummy) / 32768.0;

	return true;
}

bool OBD2::get_pid_sub40(int pid, double *ret)
{
	int dummy;

	if (!get_pid_8b(pid, &dummy))
		return false;

	*ret = double(dummy) - 40.0;

	return true;
}

bool OBD2::get_pid_double(int pid, double *value)
{
	int dummy;

	bool rc = get_pid_8b(pid, &dummy);

	*value = dummy;

	return rc;
}

bool OBD2::get_pid_16b_double(int pid, double *value)
{
	int dummy;

	bool rc = get_pid_16b(pid, &dummy);

	*value = dummy;

	return rc;
}

bool OBD2::get_pid_3B_div200(int pid, double *ret)
{
	byte_array *data = NULL;

	for(;;)
	{
		if (!is_supported[pid])
		{
			t -> emit(LL_ERROR, format("Requesting unsupported pid %02x", pid));
			break;
		}

		data = get_pid(pid);

		if (!data || data -> size() == 0)
			break;

		if (data -> size() != 4)
		{
			t -> emit(LL_ERROR, format("Unexpected reply size (%d bytes, expected 4)", data -> size()));
			break;
		}

		*ret = double(data -> get_uchar(2)) / 200.0;

		delete data;

		return true;
	}

	n_fail++;

	io -> flush();

	delete data;

	return false;
}

bool OBD2::get_pid_16b_div_1000(int pid, double *value)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*value = double(dummy) / 1000.0;

	return true;
}

bool OBD2::get_pid_16b_mul0_079(int pid, double *value)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*value = double(dummy) * 0.079;

	return true;
}

bool OBD2::get_pid_16b_mul10(int pid, double *value)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*value = double(dummy) * 10.0;

	return true;
}

bool OBD2::get_pid_16b_div10_sub40(int pid, double *ret)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*ret = double(dummy) / 10.0 - 40.0;

	return true;
}

bool OBD2::get_pid_16b_sub26880_div128(int pid, double *value)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*value = (double(dummy) - 26880.0) / 128.0;

	return true;
}

bool OBD2::get_pid_16b_mul0_05(int pid, double *value)
{
	int dummy;

	if (!get_pid_16b(pid, &dummy))
		return false;

	*value = double(dummy) * 0.05;

	return true;
}

bool OBD2::get_pid_ab_div32768(int pid, double *value)
{
	byte_array *data = get_pid(pid);

	for(;;)
	{
		if (!data || data -> size() == 0)
			break;

		if (data -> size() != 6)
		{
			t -> emit(LL_ERROR, format("Unexpected reply size (%d bytes, expected 6)", data -> size()));
			break;
		}

		*value = double((data -> get_uchar(2) << 8) + data -> get_uchar(3)) / 32768.0;

                delete data;

                return true;
        }

        delete data;

        n_fail++;

        io -> flush();

        return false;
}

bool OBD2::get_pid_cd_div8192(int pid, double *value)
{
	byte_array *data = get_pid(pid);

	for(;;)
	{
		if (!data || data -> size() == 0)
			break;

		if (data -> size() != 6)
		{
			t -> emit(LL_ERROR, format("Unexpected reply size (%d bytes, expected 6)", data -> size()));
			break;
		}

		*value = double((data -> get_uchar(4) << 8) + data -> get_uchar(5)) / 8192.0;

                delete data;

                return true;
        }

        delete data;

        n_fail++;

        io -> flush();

        return false;
}

void OBD2::get_io_stats(int *n_ok, int *n_fail, double *avg_resp_time) const
{
	*n_ok = this -> n_ok;
	*n_fail = this -> n_fail;
	*avg_resp_time = avg_ts / double(this -> n_ok);
}

bool OBD2::get_io_sensor(std::string symbol, double *value)
{
	return io -> get_sensor(symbol, value);
}

bool OBD2::get_string(int mode, int pid, std::string *out)
{
	t -> emit(LL_DEBUG_ALL, format("Requesting string from %02x:%02x", mode, pid));

	out -> clear();

	byte_array command;
	std::vector<byte_array *> replies;

	command.set(0, mode);
	command.set(1, pid);
	io -> send_recv(&command, &replies);

	if (replies.size() == 0)
	{
		t -> emit(LL_ERROR, "No reply from device");
		return false;
	}

	bool fail = false;
	std::string result;
	for(unsigned int index=0; index<replies.size(); index++)
	{
		byte_array *reply = replies.at(index);

		if (reply -> size() > 3)
		{
			for(int byte=3; byte<reply -> size(); byte++)
			{
				char b = reply -> get_char(byte);

				if (b)
					result += b;
			}
		}
		else
		{
			t -> emit(LL_ERROR, format("Truncated reply (%d bytes in size)", reply -> size()));
			fail = true;
		}

		delete replies.at(index);
	}

	if (!fail)
		out -> assign(result);
	else
	{
		io -> flush();

		n_fail++;
	}

	return !fail;
}

bool OBD2::get_VIN(std::string *out)
{
	return get_string(0x09, 0x2, out);
}

bool OBD2::get_calibration_ID(std::string *out)
{
	return get_string(0x09, 0x4, out);
}

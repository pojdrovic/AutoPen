class sensor_catalyst_temperature_bank_1_sensor_2 : public sensor_catalyst_temperature_bank_1_sensor_1
{
public:
	int get_pid() { return 0x3e; }
	std::string get_symbol() { return "catalyst_temperature_bank_1_sensor_2"; }
	std::string get_description() { return "catalyst temperature bank 1 sensor 2"; }
	color_t get_color() { return C_BLUE; }
};

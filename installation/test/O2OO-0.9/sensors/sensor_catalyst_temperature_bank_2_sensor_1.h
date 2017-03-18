class sensor_catalyst_temperature_bank_2_sensor_1 : public sensor_catalyst_temperature_bank_1_sensor_1
{
public:
	int get_pid() { return 0x3d; }
	std::string get_symbol() { return "catalyst_temperature_bank_2_sensor_1"; }
	std::string get_description() { return "catalyst temperature bank 2 sensor 1"; }
	color_t get_color() { return C_YELLOW; }
};

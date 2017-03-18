class sensor_oxygen_sensor_bank2_sensor4 : public sensor_oxygen_sensor_bank1_sensor1
{
public:
	int get_pid() { return 0x1b; }
	std::string get_symbol() { return "oxygen_sensor_bank2_sensor4"; }
	std::string get_description() { return "oxygen sensor bank 2 sensor 4"; }
	color_t get_color() { return C_WHITE; }
};

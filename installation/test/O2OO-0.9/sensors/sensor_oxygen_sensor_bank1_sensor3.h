class sensor_oxygen_sensor_bank1_sensor3 : public sensor_oxygen_sensor_bank1_sensor1
{
public:
	int get_pid() { return 0x16; }
	std::string get_symbol() { return "oxygen_sensor_bank1_sensor3"; }
	std::string get_description() { return "oxygen sensor bank 1 sensor 3"; }
	color_t get_color() { return C_GREEN; }
};

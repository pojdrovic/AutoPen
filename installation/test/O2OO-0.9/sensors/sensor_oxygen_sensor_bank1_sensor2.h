class sensor_oxygen_sensor_bank1_sensor2 : public sensor_oxygen_sensor_bank1_sensor1
{
public:
	int get_pid() { return 0x15; }
	std::string get_symbol() { return "oxygen_sensor_bank1_sensor2"; }
	std::string get_description() { return "oxygen sensor bank 1 sensor 2"; }
	color_t get_color() { return C_RED; }
};

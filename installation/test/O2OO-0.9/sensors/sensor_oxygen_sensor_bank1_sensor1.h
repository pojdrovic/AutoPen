class sensor_oxygen_sensor_bank1_sensor1 : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x14; }
	std::string get_symbol() { return "oxygen_sensor_bank1_sensor1"; }
	std::string get_unit() { return "volts"; }
	std::string get_description() { return "oxygen sensor bank 1 sensor 1"; }
	std::string get_screen_formatting() { return "%5.3fv"; }
	color_t get_color() { return C_WHITE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 1.275; }
	processing_t get_processor() { return ppid_3B_div200; }
	bool is_meta() { return false; }
};

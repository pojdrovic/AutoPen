class sensor_maximum_oxygen_sensor_voltage : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x4f; }
	std::string get_symbol() { return "maximum_oxygen_sensor_voltage"; }
	std::string get_unit() { return "V"; }
	std::string get_description() { return "maximum oxygen sensor voltage"; }
	std::string get_screen_formatting() { return "%3.0f"; }
	color_t get_color() { return C_RED; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 255; }
	processing_t get_processor() { return ppid_8b_b; }
	bool is_meta() { return true; }
};

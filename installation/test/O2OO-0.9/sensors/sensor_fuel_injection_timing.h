class sensor_fuel_injection_timing : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x5d; }
	std::string get_symbol() { return "fuel_injection_timing"; }
	std::string get_unit() { return "degrees"; }
	std::string get_description() { return "fuel injection timing"; }
	std::string get_screen_formatting() { return "%8.3f"; }
	color_t get_color() { return C_YELLOW; }
	void get_range(double *mi, double *ma) { *mi = -210; *ma = 301.992; }
	processing_t get_processor() { return ppid_16b_sub26880_div128; }
	bool is_meta() { return false; }
};

class sensor_control_module_voltage : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x42; }
	std::string get_symbol() { return "control_module_voltage"; }
	std::string get_unit() { return "V"; }
	std::string get_description() { return "control module voltage"; }
	std::string get_screen_formatting() { return "%6.3fV"; }
	color_t get_color() { return C_CYAN; }
	void get_range(double *mi, double *ma) { *mi = -40; *ma = 65.535; }
	processing_t get_processor() { return ppid_16b_div_1000; }
	bool is_meta() { return false; }
};

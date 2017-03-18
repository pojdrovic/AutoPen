class sensor_timing_advance : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x0e; }
	std::string get_symbol() { return "timing_advance"; }
	std::string get_unit() { return "degrees"; }
	std::string get_description() { return "timing advance"; }
	std::string get_screen_formatting() { return "%5.1f degrees"; }
	color_t get_color() { return C_YELLOW; }
	void get_range(double *mi, double *ma) { *mi = -64; *ma = 63.5; }
	processing_t get_processor() { return ppid_div2_sub64; }
	bool is_meta() { return false; }
};

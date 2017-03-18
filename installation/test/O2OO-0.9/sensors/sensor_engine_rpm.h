class sensor_engine_rpm : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x0c; }
	std::string get_symbol() { return "engine_rpm"; }
	std::string get_unit() { return "RPM"; }
	std::string get_description() { return "RPM"; }
	std::string get_screen_formatting() { return "%6.1f RPM"; }
	color_t get_color() { return C_RED; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 16383.75; }
	processing_t get_processor() { return ppid_16b_div4; }
	bool is_meta() { return false; }
};

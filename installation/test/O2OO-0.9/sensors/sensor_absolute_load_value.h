class sensor_absolute_load_value : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x43; }
	std::string get_symbol() { return "absolute_load_value"; }
	std::string get_unit() { return "%"; }
	std::string get_description() { return "absolute load value"; }
	std::string get_screen_formatting() { return "%5.1f%%"; }
	color_t get_color() { return C_RED; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 25700.0; }
	processing_t get_processor() { return ppid_16b_percent_25700; }
	bool is_meta() { return false; }
};

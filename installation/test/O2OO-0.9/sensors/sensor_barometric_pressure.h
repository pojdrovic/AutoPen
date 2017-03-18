class sensor_barometric_pressure : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x33; }
	std::string get_symbol() { return "barometric_pressure"; }
	std::string get_unit() { return "kPa (absolute)"; }
	std::string get_description() { return "barometric pressure"; }
	std::string get_screen_formatting() { return "%3.0fkPa (absolute)"; }
	color_t get_color() { return C_MAGENTA; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 255; }
	processing_t get_processor() { return ppid_pid_double; }
	bool is_meta() { return false; }
};

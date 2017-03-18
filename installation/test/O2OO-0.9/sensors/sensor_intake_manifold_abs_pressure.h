class sensor_intake_manifold_abs_pressure : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x0b; }
	std::string get_symbol() { return "intake_manifold_abs_pressure"; }
	std::string get_unit() { return "kPa (absolute)"; }
	std::string get_description() { return "intake manifold absolute pressure"; }
	std::string get_screen_formatting() { return "%3.0fkPa (absolute)"; }
	color_t get_color() { return C_WHITE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 255; }
	processing_t get_processor() { return ppid_pid_double; }
	bool is_meta() { return false; }
};

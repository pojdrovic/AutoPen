class sensor_maximum_intake_manifold_abs_pressure : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x4f; }
	std::string get_symbol() { return "maximum_intake_manifold_abs_pressure"; }
	std::string get_unit() { return ""; }
	std::string get_description() { return "maximum intake manifold absolute pressure"; }
	std::string get_screen_formatting() { return "%4.0f"; }
	color_t get_color() { return C_GREEN; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 255.0; }
	processing_t get_processor() { return ppid_8b_mul10_d; }
	bool is_meta() { return true; }
};

class sensor_maximum_equivalance_ratio : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x4f; }
	std::string get_symbol() { return "maximum_equivalence_ratio"; }
	std::string get_unit() { return ""; }
	std::string get_description() { return "maximum equivalence ratio"; }
	std::string get_screen_formatting() { return "%3.0f"; }
	color_t get_color() { return C_GREEN; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 255; }
	processing_t get_processor() { return ppid_8b_a; }
	bool is_meta() { return true; }
};

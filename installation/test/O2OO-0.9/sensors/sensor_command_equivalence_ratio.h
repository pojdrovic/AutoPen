class sensor_command_equivalence_ratio : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x44; }
	std::string get_symbol() { return "command_equivalence_ratio"; }
	std::string get_unit() { return "N/A"; }
	std::string get_description() { return "command equivalence ratio"; }
	std::string get_screen_formatting() { return "%7.1f"; }
	color_t get_color() { return C_MAGENTA; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 2; }
	processing_t get_processor() { return ppid_16b_div32768; }
	bool is_meta() { return false; }
};

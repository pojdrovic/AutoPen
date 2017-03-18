class sensor_o2s5_wr_lambda_equivalence_ratio : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x28; }
	std::string get_symbol() { return "o2s5_wr_lambda_equivalence_ratio"; }
	std::string get_unit() { return ""; }
	std::string get_description() { return "O2S 5 WR lambda equivalence ratio"; }
	std::string get_screen_formatting() { return "%5.3f"; }
	color_t get_color() { return C_MAGENTA; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 1.999; }
	processing_t get_processor() { return ppid_ab_div32768; }
	bool is_meta() { return false; }
};

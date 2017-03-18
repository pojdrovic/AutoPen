class sensor_o2s3_wr_lambda_voltage : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x26; }
	std::string get_symbol() { return "o2s3_wr_lambda_voltage"; }
	std::string get_unit() { return ""; }
	std::string get_description() { return "O2S 3 WR lambda voltage"; }
	std::string get_screen_formatting() { return "%5.3f"; }
	color_t get_color() { return C_GREEN; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 7.999; }
	processing_t get_processor() { return ppid_cd_div8192; }
	bool is_meta() { return false; }
};

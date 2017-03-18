class sensor_maf_air_flow_rate : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x10; }
	std::string get_symbol() { return "maf_air_flow_rate"; }
	std::string get_unit() { return "grams/sec"; }
	std::string get_description() { return "MAF air flow rate"; }
	std::string get_screen_formatting() { return "%6.2f grams/s"; }
	color_t get_color() { return C_CYAN; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 655.35; }
	processing_t get_processor() { return ppid_16b_div100; }
	bool is_meta() { return false; }
};

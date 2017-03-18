class sensor_evap_system_vapor_pressure : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x32; }
	std::string get_symbol() { return "evap_system_vapor_pressure"; }
	std::string get_unit() { return "Pa"; }
	std::string get_description() { return "evaporation system pressure"; }
	std::string get_screen_formatting() { return "%6.1fPa"; }
	color_t get_color() { return C_CYAN; }
	void get_range(double *mi, double *ma) { *mi = -8192; *ma = 8192; }
	processing_t get_processor() { return ppid_16b_div4; }
	bool is_meta() { return false; }
};

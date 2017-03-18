class sensor_long_term_fuel_bank1 : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x07; }
	std::string get_symbol() { return "long_term_fuel_bank1"; }
	std::string get_unit() { return "%"; }
	std::string get_description() { return "long term fuel bank 1"; }
	std::string get_screen_formatting() { return "%5.1f%%"; }
	color_t get_color() { return C_YELLOW; }
	void get_range(double *mi, double *ma) { *mi = -100; *ma = 99.22; }
	processing_t get_processor() { return ppid_percent128; }
	bool is_meta() { return false; }
};

class sensor_engine_coolant_temperature : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x05; }
	std::string get_symbol() { return "engine_coolant_temperature"; }
	std::string get_unit() { return "celsius"; }
	std::string get_description() { return "coolant temperature"; }
	std::string get_screen_formatting() { return "%3.0fC"; }
	color_t get_color() { return C_RED; }
	void get_range(double *mi, double *ma) { *mi = -40; *ma = 215; }
	processing_t get_processor() { return ppid_sub40; }
	bool is_meta() { return false; }
};

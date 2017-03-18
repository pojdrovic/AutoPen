class sensor_distance_traveled_since_codes_cleared : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x31; }
	std::string get_symbol() { return "distance_traveled_since_codes_cleared"; }
	std::string get_unit() { return "km"; }
	std::string get_description() { return "distance traveled"; }
	std::string get_screen_formatting() { return "%5.0fkm"; }
	color_t get_color() { return C_WHITE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 65535; }
	processing_t get_processor() { return ppid_16b_double; }
	bool is_meta() { return false; }
};

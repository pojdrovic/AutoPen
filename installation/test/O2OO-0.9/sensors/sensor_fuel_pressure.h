class sensor_fuel_pressure : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x0a; }
	std::string get_symbol() { return "fuel_pressure"; }
	std::string get_unit() { return "kPa (gauge)"; }
	std::string get_description() { return "fuel pressure"; }
	std::string get_screen_formatting() { return "%3.0fkPa (gauge)"; }
	color_t get_color() { return C_MAGENTA; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 765; }
	processing_t get_processor() { return ppid_mul3; }
	bool is_meta() { return false; }
};

class sensor_fuel_rail_pressure_direct_inject : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x23; }
	std::string get_symbol() { return "fuel_rail_pressure_direct_inject"; }
	std::string get_unit() { return "kPa (gauge)"; }
	std::string get_description() { return "fuel rail pressure direct inject"; }
	std::string get_screen_formatting() { return "%3.0fkPa (gauge)"; }
	color_t get_color() { return C_BLUE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 655.350; }
	processing_t get_processor() { return ppid_16b_mul10; }
	bool is_meta() { return false; }
};

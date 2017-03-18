class sensor_fuel_rail_pressure_manifold_vacuum : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x22; }
	std::string get_symbol() { return "fuel_rail_pressure_manifold_vacuum"; }
	std::string get_unit() { return "kPa"; }
	std::string get_description() { return "fuel rail pressure manifold vacuum"; }
	std::string get_screen_formatting() { return "%3.0fkPa"; }
	color_t get_color() { return C_YELLOW; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 5177.265; }
	processing_t get_processor() { return ppid_16b_mul0_079; }
	bool is_meta() { return false; }
};

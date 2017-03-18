class sensor_engine_fuel_rate : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x5e; }
	std::string get_symbol() { return "engine_fuel_rate"; }
	std::string get_unit() { return "L/h"; }
	std::string get_description() { return "engine fuel rate"; }
	std::string get_screen_formatting() { return "%5.1fL/h"; }
	color_t get_color() { return C_BLUE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 3212.75; }
	processing_t get_processor() { return ppid_16b_mul0_05; }
	bool is_meta() { return false; }
};

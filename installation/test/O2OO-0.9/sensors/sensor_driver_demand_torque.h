class sensor_driver_demand_torque : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x61; }
	std::string get_symbol() { return "driver_demand_torque"; }
	std::string get_unit() { return "%"; }
	std::string get_description() { return "driver demand torgue"; }
	std::string get_screen_formatting() { return "%5.1f%%"; }
	color_t get_color() { return C_CYAN; }
	void get_range(double *mi, double *ma) { *mi = -125; *ma = 125; }
	processing_t get_processor() { return ppid_sub125; }
	bool is_meta() { return false; }
};

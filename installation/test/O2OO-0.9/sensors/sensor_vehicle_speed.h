class sensor_vehicle_speed : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x0d; }
	std::string get_symbol() { return "vehicle_speed"; }
	std::string get_unit() { return "km/h"; }
	std::string get_description() { return "speed"; }
	std::string get_screen_formatting() { return "%3.0fkm/h"; }
	color_t get_color() { return C_GREEN; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 255; }
	processing_t get_processor() { return ppid_pid_double; }
	bool is_meta() { return false; }
};

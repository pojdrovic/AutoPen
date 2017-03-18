class io_sensor_battery_voltage : public io_sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x01; }
	std::string get_symbol() { return "battery_voltage"; }
	std::string get_unit() { return "V"; }
	std::string get_description() { return "battery voltage"; }
	std::string get_screen_formatting() { return "%2.1fV"; }
	color_t get_color() { return C_GREEN; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 24.0; }
	processing_t get_processor() { return io_get; }
	bool is_meta() { return false; }
};

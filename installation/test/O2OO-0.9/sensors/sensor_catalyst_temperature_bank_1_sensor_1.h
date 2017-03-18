class sensor_catalyst_temperature_bank_1_sensor_1 : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x3c; }
	std::string get_symbol() { return "catalyst_temperature_bank_1_sensor_1"; }
	std::string get_unit() { return "celsius"; }
	std::string get_description() { return "catalyst temperature bank 1 sensor 1"; }
	std::string get_screen_formatting() { return "%6.1fC"; }
	color_t get_color() { return C_GREEN; }
	void get_range(double *mi, double *ma) { *mi = -40; *ma = 6513.5; }
	processing_t get_processor() { return ppid_16b_div10_sub40; }
	bool is_meta() { return false; }
};

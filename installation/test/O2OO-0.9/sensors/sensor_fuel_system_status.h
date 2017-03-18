class sensor_fuel_system_status : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x03; }
	std::string get_symbol() { return "fuel_system_status"; }
	std::string get_unit() { return "bit encoded"; }
	std::string get_description() { return "fuel system status"; }
	std::string get_screen_formatting() { return "%s"; }
	color_t get_color() { return C_WHITE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 0; }
	processing_t get_processor() { return ppid_string; }
	bool is_meta() { return true; }
	bool get_value(OBD2 *o, std::string *value);
};

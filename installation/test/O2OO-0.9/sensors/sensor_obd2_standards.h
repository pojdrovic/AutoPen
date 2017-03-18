class sensor_obd2_standards : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x1c; }
	std::string get_symbol() { return "obd2_standards"; }
	std::string get_unit() { return "string"; }
	std::string get_description() { return "OBD2 standards complying to"; }
	std::string get_screen_formatting() { return "%s"; }
	color_t get_color() { return C_WHITE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 0; }
	processing_t get_processor() { return ppid_string; }
	bool is_meta() { return true; }
	bool get_value(OBD2 *o, std::string *value);
};

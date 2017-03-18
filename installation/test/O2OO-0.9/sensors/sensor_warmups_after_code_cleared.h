class sensor_warmups_after_code_cleared : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x30; }
	std::string get_symbol() { return "warmups_after_code_cleared"; }
	std::string get_unit() { return "#"; }
	std::string get_description() { return "# of warm-ups since codes cleared"; }
	std::string get_screen_formatting() { return "%s"; }
	color_t get_color() { return C_WHITE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 0; }
	processing_t get_processor() { return ppid_string; }
	bool is_meta() { return true; }
	bool get_value(OBD2 *o, std::string *value);
};

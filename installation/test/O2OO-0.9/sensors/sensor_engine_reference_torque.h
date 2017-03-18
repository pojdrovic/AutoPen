class sensor_engine_reference_torque : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x63; }
	std::string get_symbol() { return "sensor_engine_reference_torque"; }
	std::string get_unit() { return "Nm"; }
	std::string get_description() { return "engine reference torque"; }
	std::string get_screen_formatting() { return "%5.0fNm"; }
	color_t get_color() { return C_WHITE; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 65535; }
	processing_t get_processor() { return ppid_16b; }
	bool is_meta() { return false; }
};

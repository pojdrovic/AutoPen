class sensor_accelerator_pedal_position_f : public sensor
{
public:
	int get_mode() { return 0x01; }
	int get_pid() { return 0x4b; }
	std::string get_symbol() { return "accelerator_pedal_position_f"; }
	std::string get_unit() { return "%"; }
	std::string get_description() { return "accelerator pedal position F"; }
	std::string get_screen_formatting() { return "%5.1f%%"; }
	color_t get_color() { return C_CYAN; }
	void get_range(double *mi, double *ma) { *mi = 0; *ma = 100; }
	processing_t get_processor() { return ppid_percent; }
	bool is_meta() { return false; }
};

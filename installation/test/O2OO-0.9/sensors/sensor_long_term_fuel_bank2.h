class sensor_long_term_fuel_bank2 : public sensor_long_term_fuel_bank1
{
public:
	int get_pid() { return 0x09; }
	std::string get_symbol() { return "long_term_fuel_bank2"; }
	std::string get_description() { return "long term fuel bank 2"; }
	color_t get_color() { return C_CYAN; }
};

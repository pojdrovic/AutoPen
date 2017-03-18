class sensor_short_term_fuel_bank2 : public sensor_short_term_fuel_bank1
{
public:
	int get_pid() { return 0x08; }
	std::string get_symbol() { return "short_term_fuel_bank2"; }
	std::string get_description() { return "short term fuel bank 2"; }
	color_t get_color() { return C_BLUE; }
};

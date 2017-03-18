class terminal_console : public terminal
{
private:

public:
	terminal_console(int verbosity, bool colors, std::string log_file);
	~terminal_console();

	void recreate_terminal() { };
	void update_terminal() { }

	void emit(int ll, std::string what);

	void set_status(std::string s) { dolog(LL_DEBUG_SOME, s); }

	void allocate_space(int width, color_t c, std::string symbol) { }
	void put(std::string symbol, std::string what) { }
};

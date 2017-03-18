class terminal_ncurses : public terminal
{
private:
	unsigned int line_usage[256]; // 256 is number of pids
	unsigned int max_x, max_y, top_n, bottom_n;
	WINDOW *top_win, *line_win, *bottom_win;
	pthread_mutex_t lck; // ncurses is not thread safe

	// symbol, location
	std::map<std::string, location_t *> sensor_map;

	std::string last_status;

	void determine_terminal_size(unsigned int *max_y, unsigned int *max_x);
	void place_sensors();
	void create_windows();
	void draw_status();

public:
	terminal_ncurses(int verbosity_in, bool colors_in, std::string log_file_in);
	~terminal_ncurses();

	void recreate_terminal();
	void update_terminal();

	void emit(int ll, std::string what);

	void set_status(std::string s);

	void allocate_space(int width, color_t c, std::string symbol);
	void put(std::string symbol, std::string what);
};

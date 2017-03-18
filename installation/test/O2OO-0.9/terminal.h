typedef enum { C_WHITE = 0, C_GREEN, C_YELLOW, C_BLUE, C_MAGENTA, C_CYAN, C_RED } color_t;

#define LL_ERROR	0
#define LL_INFO		1
#define LL_DEBUG_SOME	2
#define LL_DEBUG_ALL	3

typedef struct
{
	int x, y;
	unsigned int width;
	color_t c;
} location_t;

class terminal
{
protected:
	int verbosity, colors;
	std::string log_file;

	void dolog(int ll, std::string what);

public:
	terminal(int verbosity, bool colors_in, std::string log_file);
	virtual ~terminal();

	virtual void recreate_terminal() = 0;
	virtual void update_terminal() = 0;

	// logging
	virtual void emit(int ll, std::string what) = 0;

	virtual void set_status(std::string s) = 0;

	// measured values
	virtual void allocate_space(int width, color_t c, std::string symbol) = 0;
	virtual void put(std::string symbol, std::string what) = 0;
};

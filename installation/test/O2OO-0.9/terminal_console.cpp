#include <iostream>
#include <string>

#include "terminal.h"
#include "terminal_console.h"

terminal_console::terminal_console(int verbosity_in, bool colors_in, std::string log_file_in) : terminal(verbosity_in, colors_in, log_file_in)
{
}

terminal_console::~terminal_console()
{
}

void terminal_console::emit(int ll, std::string what)
{
	if (ll <= verbosity)
		std::cout << what << std::endl;

	dolog(ll, what);
}

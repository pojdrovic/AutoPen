#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>

#include "error.h"
#include "terminal.h"

terminal::terminal(int verbosity_in, bool colors_in, std::string log_file_in) : verbosity(verbosity_in), colors(colors_in), log_file(log_file_in)
{
}

terminal::~terminal()
{
}

void terminal::dolog(int ll, std::string what)
{
	if (log_file != "")
	{
		time_t now = time(NULL);
		char timestr[256] = { 0 };
		ctime_r(&now, timestr); // can't set a limit so hopefully this 256 is enough!
		char *dummy = strchr(timestr, '\n');
		if (dummy) *dummy = 0x00;

		FILE *fh = fopen(log_file.c_str(), "a+");
		if (!fh)
			error_exit("Cannot create file %s", log_file.c_str());

		const char *ll_str = "?";
		if (ll == LL_ERROR)
			ll_str = "ERROR";
		else if (ll == LL_INFO)
			ll_str = "INFO ";
		else if (ll == LL_DEBUG_SOME)
			ll_str = "DSOME";
		else if (ll == LL_DEBUG_ALL)
			ll_str = "DALL ";

		fprintf(fh, "%s %s %s\n", timestr, ll_str, what.c_str());

		fclose(fh);
	}
}

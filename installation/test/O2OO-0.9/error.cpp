// SVN: $Revision: 442 $
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <syslog.h>
#include <ncurses.h>
#include <execinfo.h>

void error_exit(const char *format, ...)
{
	char buffer[4096];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buffer, sizeof buffer, format, ap);
	va_end(ap);

	fprintf(stderr, "\n\nERROR: %s\n\n\n", buffer);

	fprintf(stderr, "Debug information:\n");
	fprintf(stderr, "errno at that time: %d (%s)\n", errno, strerror(errno));

	void *trace[128];
	int trace_size = backtrace(trace, 128);
	char **messages = backtrace_symbols(trace, trace_size);
	fprintf(stderr, "Execution path:");
	for(int index=0; index<trace_size; ++index)
		fprintf(stderr, "%d %s\n", index, messages[index]);

	exit(EXIT_FAILURE);
}

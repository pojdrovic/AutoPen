// SVN: $Revision: 459 $
#include <arpa/inet.h>
#include <errno.h>
#include <math.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include "error.h"
#include "utils.h"

std::string format(const char *fmt, ...)
{
	char *buffer = NULL;
        va_list ap;

        va_start(ap, fmt);
        (void)vasprintf(&buffer, fmt, ap);
        va_end(ap);

	std::string result = buffer;
	free(buffer);

	return result;
}

void split_string(const char *in, const char *split, char ***out, int *n_out)
{
	int split_len = strlen(split);
	char *copy_in = strdup(in), *dummy = copy_in;

	for(;;)
	{
		char *next = NULL;

		(*n_out)++;
		*out = reinterpret_cast<char **>(realloc(*out, *n_out * sizeof(char *)));

		next = strstr(copy_in, split);
		if (!next)
		{
			(*out)[*n_out - 1] = strdup(copy_in);
			break;
		}

		*next = 0x00;

		(*out)[*n_out - 1] = strdup(copy_in);

		copy_in = next + split_len;
	}

	free(dummy);
}

std::vector<std::string> split_string(std::string in, std::string split)
{
	char **out = NULL;
	int n_out = 0;

	split_string(in.c_str(), split.c_str(), &out, &n_out);

	std::vector<std::string> list_out;

	for(int index=0; index<n_out; index++)
	{
		list_out.push_back(out[index]);
		free(out[index]);
	}

	free(out);

	return list_out;
}

int hex_to_int(std::string in)
{
	int value = 0;

	for(unsigned int index=0; index<in.size(); index++)
	{
		value <<= 4;

		if (in.at(index) >= '0' && in.at(index) <= '9')
			value += in.at(index) - '0';
		else if (in.at(index) >= 'A' && in.at(index) <= 'F')
			value += in.at(index) - 'A' + 10;
		else if (in.at(index) >= 'a' && in.at(index) <= 'f')
			value += in.at(index) - 'a' + 10;
		else
			return -1;
	}

	return value;
}

int WRITE(int fd, const char *whereto, size_t len)
{
        ssize_t cnt=0;

        while(len>0)
        {
                ssize_t rc = write(fd, whereto, len);

                if (rc == -1)
                {
                        if (errno != EINTR && errno != EINPROGRESS && errno != EAGAIN)
                                return -1;

			continue;
                }

                if (rc == 0)
                        return -1;

		whereto += rc;
		len -= rc;
		cnt += rc;
	}

	return cnt;
}

double get_ts()
{
        struct timeval ts;

        if (gettimeofday(&ts, NULL) == -1)
                error_exit("gettimeofday failed");

        return double(ts.tv_sec) + double(ts.tv_usec) / 1000000.0;
}

std::string trim(std::string in)
{
	size_t len = in.size();
	size_t index=0;
	while(index<len && isspace(in.at(index)))
		index++;
	if (index == len)
		return "";
	size_t start = index;

	size_t end = len - 1;
	while(end > start && isspace(in.at(end)))
		end--;

	return in.substr(start, end - start + 1);
}

std::string get_time_str()
{
	time_t now = time(NULL);
	char *now_str = ctime(&now);
	char *dummy = strchr(now_str, '\n');
	if (dummy)
		*dummy = 0x00;

	return now_str;
}

std::string time_to_str(time_t t)
{
        if (t <= 0)
                return "n/a";

        struct tm *tm = localtime(&t);
	if (!tm)
		error_exit("localtime(%ld) failed", (long int)t);

        char time_buffer[128];
        strftime(time_buffer, sizeof time_buffer, "%a, %d %b %Y %T %z", tm);

        return std::string(time_buffer);
}

std::string time_to_str(double t_in)
{
        if (t_in <= 0)
                return "n/a";

	time_t t = (time_t)t_in;
        struct tm *tm = localtime(&t);
	if (!tm)
		error_exit("localtime(%ld) failed", (long int)t);

        char time_buffer[128];
	snprintf(time_buffer, sizeof time_buffer, "%04d-%02d-%02dT%02d:%02d:%02d.%03d",
		tm -> tm_year + 1900,
		tm -> tm_mon + 1,
		tm -> tm_mday,
		tm -> tm_hour,
		tm -> tm_min,
		tm -> tm_sec,
		int((t_in - double(t)) * 1000.0) % 1000);

        return std::string(time_buffer);
}

// taken from http://stackoverflow.com/questions/365826/calculate-distance-between-2-gps-coordinates
// assuming lat1,long1 = cur
// calculate haversine distance for linear distance
double haversine_km(double lat1, double long1, double lat2, double long2)
{
	double d2r = PI / 180.0;
	double dlong = (long2 - long1) * d2r;
	double dlat = (lat2 - lat1) * d2r;
	double a = pow(sin(dlat/2.0), 2.0) + cos(lat1*d2r) * cos(lat2*d2r) * pow(sin(dlong/2.0), 2.0);
	double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

	// was 6367, replaces by "Radius at a given geodetic latitude" from http://en.wikipedia.org/wiki/Earth_radius
	double r_a = 6384.4;
	double r_b = 6352.8;
	double glang_cos = cos(lat1 * d2r);
	double glang_sin = sin(lat1 * d2r);
	double r1 = pow(pow(r_a, 2.0) * glang_cos, 2.0) + pow(pow(r_b, 2.0) * glang_sin, 2.0);
	double r2 = pow(r_a * glang_cos, 2.0) + pow(r_b * glang_sin, 2.0);
	double r = sqrt(r1 / r2);
	double d = r * c;

	return d;
}

std::string shorten(double value)
{
	double chk = fabs(value);

	double divider = 1.0;
	std::string si;
	if (chk >= 1000000000000.0)
	{
		si = "T";
		divider = 1000000000000.0;
	}
	else if (chk >= 1000000000.0)
	{
		si = "G";
		divider = 1000000000.0;
	}
	else if (chk >= 1000000.0)
	{
		si = "M";
		divider = 1000000.0;
	}
	else if (chk >= 1000.0)
	{
		si = "k";
		divider = 1000.0;
	}
	else if (chk == 0.0)
		return "0";
	else if (chk <= 0.000001)
	{
		si = "u";
		divider = 0.000001;
	}
	else if (chk <= 0.001)
	{
		si = "m";
		divider = 0.001;
	}

	std::string dummy = format("%.0f", value / divider);
	int len = dummy.length();
	if (len < 3)
	{
		std::string fmt = "%." + format("%d", 3 - len) + "f";
		dummy = format(fmt.c_str(), value / divider);
	}

	return dummy + si;
}

int sl_callback(void *cb_data, int argc, char **argv, char **column_names)
{
	std::vector<std::string> row;

	for(int index=0; index<argc; index++)
	{
		if (argv[index])
			row.push_back(argv[index]);
		else
			row.push_back("");
	}

	std::vector<std::vector<std::string> > *result_set = (std::vector<std::vector<std::string> > *)cb_data;

	result_set -> push_back(row);

	return 0;
}

int get_query_value(sqlite3 *db, std::string query)
{
        std::vector<std::vector<std::string> > results;
        char *error = NULL;

        if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
                error_exit("DB error: %s", error);

	return atoi(results.at(0).at(0).c_str());
}

bool check_table_exists(sqlite3 *db, std::string table)
{
	if (get_query_value(db, "SELECT COUNT(*) AS n FROM sqlite_master WHERE type='table' AND name='" + table + "'") == 1)
		return true;

	return false;
}

std::string read_text_file(std::string file)
{
	FILE *fh = fopen(file.c_str(), "r");
	if (!fh)
		error_exit("Problem opening file %s", file.c_str());
	int fd = fileno(fh);

	struct stat st;
	if (fstat(fd, &st) == -1)
		error_exit("Stat on file %s failed", file.c_str());

	char *buffer = (char *)malloc(st.st_size + 1);
	if (fread(buffer, 1, st.st_size, fh) != st.st_size)
		error_exit("Short read on %s", file.c_str());

	fclose(fh);

	buffer[st.st_size] = 0x00;

	return std::string(buffer);
}

void start_transaction(sqlite3 *db)
{
	char *error = NULL;
	if (sqlite3_exec(db, "begin", NULL, NULL, &error))
		error_exit("DB error: %s", error);
}

void commit_transaction(sqlite3 *db)
{
	char *error = NULL;
	if (sqlite3_exec(db, "commit", NULL, NULL, &error))
		error_exit("DB error: %s", error);
}

int connect_to(const char *host, int portnr)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    // Allow IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;    // For wildcard IP address
	hints.ai_protocol = 0;          // Any protocol
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	char portnr_str[8];
	snprintf(portnr_str, sizeof portnr_str, "%d", portnr);

	struct addrinfo *result;
	int rc = getaddrinfo(host, portnr_str, &hints, &result);
	if (rc != 0)
		error_exit("Problem resolving %s: %s\n", host, gai_strerror(rc));

	for(struct addrinfo *rp = result; rp != NULL; rp = rp->ai_next)
	{
		int fd = socket(rp -> ai_family, rp -> ai_socktype, rp -> ai_protocol);
		if (fd == -1)
			continue;

		if (connect(fd, rp -> ai_addr, rp -> ai_addrlen) == 0)
		{
			freeaddrinfo(result);

			return fd;
		}

		close(fd);
	}

	freeaddrinfo(result);

	return -1;
}

int start_listen(const char *adapter, int portnr)
{
        int fd = socket(AF_INET6, SOCK_STREAM, 0);
        if (fd == -1)
                error_exit("failed creating socket");

#ifdef TCP_TFO
	int qlen = 5;
	if (setsockopt(fd, SOL_TCP, TCP_FASTOPEN, &qlen, sizeof(qlen)) == -1)
		error_exit("Setting TCP_FASTOPEN on server socket failed");
#endif

        int reuse_addr = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char *>(&reuse_addr), sizeof reuse_addr) == -1)
                error_exit("setsockopt(SO_REUSEADDR) failed");

        struct sockaddr_in6 server_addr;

        int server_addr_len = sizeof server_addr;

        memset(reinterpret_cast<char *>(&server_addr), 0x00, server_addr_len);
        server_addr.sin6_family = AF_INET6;
        server_addr.sin6_port = htons(portnr);

        if (!adapter || strcmp(adapter, "0.0.0.0") == 0)
                server_addr.sin6_addr = in6addr_any;
        else if (inet_pton(AF_INET6, adapter, &server_addr.sin6_addr) == 0)
	{
		fprintf(stderr, "\n");
		fprintf(stderr, " * inet_pton(%s) failed: %s\n", adapter, strerror(errno));
		fprintf(stderr, " * If you're trying to use an IPv4 address (e.g. 192.168.0.1 or so)\n");
		fprintf(stderr, " * then do not forget to place ::FFFF: in front of the address,\n");
		fprintf(stderr, " * e.g.: ::FFFF:192.168.0.1\n\n");
		error_exit("listen socket initialisation failure: did you configure a correct listen adapter? (run with -n for details)");
	}

        if (bind(fd, (struct sockaddr *)&server_addr, server_addr_len) == -1)
                error_exit("bind([%s]:%d) failed", adapter, portnr);

        if (listen(fd, 5) == -1)
                error_exit("listen(%d) failed", 5);

	return fd;
}

void disable_nagle(int fd)
{
	int disable = 1;

	// EBADF might happen if a connection was closed just before this call
	if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&disable), sizeof disable) == -1 && errno != EBADF)
		error_exit("setsockopt(IPPROTO_TCP, TCP_NODELAY) failed (fd: %d)", fd);
}

std::string get_endpoint_name(int fd)
{
	char buffer[4096] = { "?" };
	struct sockaddr_in6 addr;
	socklen_t addr_len = sizeof addr;

	if (getpeername(fd, (struct sockaddr *)&addr, &addr_len) == -1)
		snprintf(buffer, sizeof buffer, "[FAILED TO FIND NAME OF %d: %s (1)]", fd, strerror(errno));
	else
	{
		char buffer2[4096];

		if (inet_ntop(AF_INET6, &addr.sin6_addr, buffer2, sizeof buffer2))
			snprintf(buffer, sizeof buffer, "[%s]:%d", buffer2, ntohs(addr.sin6_port));
		else
			snprintf(buffer, sizeof buffer, "[FAILED TO FIND NAME OF %d: %s (1)]", fd, strerror(errno));
	}

	return std::string(buffer);
}

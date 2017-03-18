// SVN: $Revision: 451 $
#include <sqlite3.h>

#define mymin(x, y)       ((x)<(y)?(x):(y))
#define mymax(x, y)       ((x)>(y)?(x):(y))

#define PI (4.0 * atan(1.0))

std::string format(const char *fmt, ...);
std::vector<std::string> split_string(std::string in, std::string split);
int hex_to_int(std::string in);
int WRITE(int fd, const char *whereto, size_t len);
double get_ts();
std::string trim(std::string in);
std::string get_time_str();
std::string time_to_str(time_t t);
std::string time_to_str(double t_in);
double haversine_km(double lat1, double long1, double lat2, double long2);
std::string shorten(double value);
int sl_callback(void *cb_data, int argc, char **argv, char **column_names);
int get_query_value(sqlite3 *db, std::string query);
bool check_table_exists(sqlite3 *db, std::string table);
std::string read_text_file(std::string file);
void start_transaction(sqlite3 *db);
void commit_transaction(sqlite3 *db);
int connect_to(const char *host, int portnr);
int start_listen(const char *adapter, int portnr);
void disable_nagle(int fd);
std::string get_endpoint_name(int fd);

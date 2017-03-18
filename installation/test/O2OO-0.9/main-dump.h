extern bool verbose;

int sl_callback(void *cb_data, int argc, char **argv, char **column_names);
void retrieve_metadata(sqlite3 *db, std::string sensor, std::string *units, double *mi, double *ma, std::string *descr);

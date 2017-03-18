#include <stdlib.h>
#include <string>
#include <vector>
#include <sqlite3.h>

#include "error.h"
#include "main.h"
#include "utils.h"

void retrieve_metadata(sqlite3 *db, std::string sensor, std::string *units, double *mi, double *ma, std::string *str)
{
	if (!db)
		error_exit("No database selected! (-b)");

	std::string query = "SELECT units, min, max, descr FROM " SENSOR_META_DATA " WHERE table_name='" + sensor +"' ORDER BY table_name";

	std::vector<std::vector<std::string> > results;
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	if (results.size() != 1)
		error_exit("Unexpected number of rows (%d) while retrieving meta-data for %s (1 row expected).\nThis is probably caused by an unknown sensor.", results.size(), sensor.c_str());

	if (results.at(0).size() != 4)
		error_exit("Unexpected number of columns (%d) while retrieving meta-data for %s (4 columns expected)", results.at(0).size(), sensor.c_str());

	std::vector<std::string> row0 = results.at(0);

	units -> assign(row0.at(0));
	*mi = atof(row0.at(1).c_str());
	*ma = atof(row0.at(2).c_str());
	str -> assign(row0.at(3));
}

void find_data_bw(sqlite3 *db, std::string sensor, double *mi, double *ma)
{
	if (!db)
		error_exit("No database selected! (-b)");

	std::string query = "SELECT MIN(value) AS mi, MAX(value) AS ma FROM " + sensor;

	std::vector<std::vector<std::string> > results;
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	if (results.size() != 1)
		error_exit("Unexpected number of rows (%d) while retrieving min/max-data for %s (1 row expected).\nThis is probably caused by an unknown sensor.", results.size(), sensor.c_str());

	if (results.at(0).size() != 2)
		error_exit("Unexpected number of columns (%d) while retrieving min/max-data for %s (2 columns expected)", results.at(0).size(), sensor.c_str());

	std::vector<std::string> row0 = results.at(0);

	*mi = atof(row0.at(0).c_str());
	*ma = atof(row0.at(1).c_str());
}

// SVN: $Revision: 442 $
#include <iostream>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <gd.h>
#include <sqlite3.h>

#include "error.h"
#include "utils.h"
#include "utils-sp.h"
#include "graph.h"
#include "kml.h"
#include "xml.h"
#include "main.h"

bool verbose = false;
std::string field_seperator = "\t";
#define DEFAULT_IMG_WIDTH 640
#define DEFAULT_IMG_HEIGHT 240
#define DEFAULT_IMG_FILE "sensor-data.png"
#define DEFAULT_IMG_FONT FONT_TEXT

void graph_sensor(sqlite3 *db, std::string sensor, std::string parameters)
{
	if (verbose)
		std::cout << "Dump of sensor " << sensor << std::endl << std::endl;

	if (!db)
		error_exit("No database selected! (-b)");

	int width = DEFAULT_IMG_WIDTH;
	int height = DEFAULT_IMG_HEIGHT;
	std::string file = DEFAULT_IMG_FILE;
	std::string font = DEFAULT_IMG_FONT;

	if (parameters.size() > 0)
	{
		std::vector<std::string> pars = split_string(parameters, ",");
		for(unsigned int index=0; index<pars.size(); index++)
		{
			std::vector<std::string> fields = split_string(pars.at(index), "=");

			if (fields.size() != 2)
				error_exit("Parameter %s is malformed (expected: x=y)", pars.at(index).c_str());

			if (fields.at(0) == "width")
				width = atoi(fields.at(1).c_str());
			if (fields.at(0) == "height")
				height = atoi(fields.at(1).c_str());
			if (fields.at(0) == "file")
				file = fields.at(1);
			if (fields.at(0) == "font")
				font = fields.at(1);
		}
	}

	if (file == DEFAULT_IMG_FILE)
		std::cout << "Please note: storing output in file " << file << std::endl;

	char *db_error = NULL;

	// retrieve meta data
	std::string units;
	double mi = 0.0, ma = 0.0;
	std::string descr;
	retrieve_metadata(db, sensor, &units, &mi, &ma, &descr);

	if (verbose)
	{
		std::cout << "File to write to: " << file << std::endl;
		std::cout << "Image dimensions: " << width << "x" << height << std::endl;
		std::cout << "Font file to use: " << font << std::endl;
		std::cout << "Units           : " << units << std::endl;
		std::cout << "Descr           : " << descr << std::endl;
	}

	// retrieve data
	std::string query = "SELECT ts, AVG(value) FROM " + sensor + " GROUP BY ts ORDER BY ts ASC";

	std::vector<std::vector<std::string> > results;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &db_error))
		error_exit("DB error: %s", db_error);

	long int *t = (long int *)malloc(sizeof(long int) * results.size());
	double *v = (double *)malloc(sizeof(double) * results.size());

	if (!t || !v)
		error_exit("Memory allocation error");

	for(unsigned int index=0; index<results.size(); index++)
	{
		std::vector<std::string> *row = &results.at(index);

		if (row -> size() != 2)
			error_exit("Problem: unexpected (%d) number of columns (expected: 2)", row -> size());

		t[index] = atoll(row -> at(0).c_str());
		v[index] = atof(row -> at(1).c_str());
	}

	graph *g = new graph(font);

	char *img_data = NULL;
	size_t img_data_len = 0;
	g -> do_draw(width, height, sensor + " (units: " + units + ")", t, v, results.size(), &img_data, &img_data_len);

	if (!img_data)
		error_exit("Unknown problem generating image %s", file.c_str());

	delete g;

	FILE *fh = fopen(file.c_str(), "wb");
	if (!fh)
		error_exit("Failed to create file %s", file.c_str());

	if (fwrite(img_data, 1, img_data_len, fh) != img_data_len)
		error_exit("Could not write all %d bytes to file %s", img_data_len, file.c_str());

	fclose(fh);

	free(img_data);
}

void graph_all_sensors(sqlite3 *db, std::string parameters)
{
	std::string query = "SELECT table_name FROM " SENSOR_META_DATA " ORDER BY table_name";

	std::vector<std::vector<std::string> > results;
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	for(unsigned int index=0; index<results.size(); index++)
	{
		std::vector<std::string> *row = &results.at(index);
		std::string table_name = row -> at(0);

		if (verbose)
			std::cout << "Drawing graph for " + table_name + " to " + table_name + ".png";

		std::string file = parameters + std::string(parameters.size() > 0 ? "," : "") + "file=" + table_name + ".png";
		graph_sensor(db, table_name, file);
	}
}

void dump_sensor(sqlite3 *db, std::string sensor, std::string parameters)
{
	if (verbose)
		std::cout << "Dump of sensor " << sensor << std::endl << std::endl;

	bool round = false;
	std::vector<std::string> pars = split_string(parameters, "=");
	if (pars.size() == 2 && pars.at(0) == "round")
		round = pars.at(1) == "true";

	if (!db)
		error_exit("No database selected! (-b)");

	std::string query = "SELECT ts, value FROM " + sensor + " ORDER BY nr ASC";

	std::vector<std::vector<std::string> > results;
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	if (verbose)
		std::cout << "timestamp" << field_seperator << "value" << std::endl;

	for(unsigned int index=0; index<results.size(); index++)
	{
		std::vector<std::string> *row = &results.at(index);

		if (round)
			std::cout << (int)atol(row -> at(0).c_str()) << field_seperator << row -> at(1) << std::endl;
		else
			std::cout << row -> at(0) << field_seperator << row -> at(1) << std::endl;
	}
}

void emit_gps(sqlite3 *db)
{
	std::string query = "SELECT ts, longitude, latitude, altitude, n_sats, ground_speed FROM " GPS_TABLE " ORDER BY nr";

	std::vector<std::vector<std::string> > results;
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	if (verbose)
		std::cout << "timestamp" << field_seperator << "longitude" << field_seperator << "latitude" << field_seperator << "altitude" << field_seperator << "# satellites" << field_seperator << "ground speed" << std::endl;

	for(unsigned int index=0; index<results.size(); index++)
	{
		std::vector<std::string> *row = &results.at(index);

		for(unsigned int col=0; col < row -> size(); col++)
		{
			if (col)
				std::cout << field_seperator;

			std::cout << row -> at(col);
		}

		std::cout << std::endl;
	}
}

void dump_metadata(sqlite3 *db)
{
	if (verbose)
		std::cout << "Dump of metadata" << std::endl << std::endl;

	if (!db)
		error_exit("No database selected! (-b)");

	char *error = NULL;
	if (check_table_exists(db, META_DATA))
	{
		std::string query = "SELECT key, value FROM " META_DATA " ORDER BY key";
		std::vector<std::vector<std::string> > results_meta_global;

		if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results_meta_global, &error))
			error_exit("DB error: %s", error);

		if (verbose)
		{
			std::cout << "key" << field_seperator << "value" << std::endl;
			std::cout << "---" << field_seperator << "-----" << std::endl;
		}
		
		for(unsigned int index=0; index<results_meta_global.size(); index++)
		{
			std::vector<std::string> *row = &results_meta_global.at(index);

			std::cout << row -> at(0) << field_seperator << row -> at(1) << std::endl;
		}
	}
	else if (verbose)
	{
		std::cout << "This database has no " META_DATA " table, skipping" << std::endl;
	}

	std::vector<std::vector<std::string> > results;
	std::string query = "SELECT table_name, units, min, max, descr FROM " SENSOR_META_DATA " ORDER BY table_name";

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	if (verbose)
		std::cout << std::endl;

	if (verbose)
	{
		std::cout << "sensor/table" << field_seperator << "units" << field_seperator << "min" << field_seperator << "max" << std::endl;
		std::cout << "------------" << field_seperator << "-----" << field_seperator << "---" << field_seperator << "---" << std::endl;
	}

	for(unsigned int index=0; index<results.size(); index++)
	{
		std::vector<std::string> *row = &results.at(index);

		for(unsigned int col=0; col < row -> size(); col++)
		{
			if (col)
				std::cout << field_seperator;

			std::cout << row -> at(col);
		}

		std::cout << std::endl;
	}
}

void help(void)
{
	std::cerr << "-b x    database (SQLite) to read from" << std::endl;
	std::cerr << "-s x    select a sensor to work on, to get a list use -c metadata (first column)" << std::endl;
	std::cerr << "-p x    set (a) parameter(s) for a command (-c)" << std::endl;
	std::cerr << "-c x    dumpmetadata: list all metadata" << std::endl;
	std::cerr << "        dumpsensor  : list all measured data of a sensor (which must be selected using -s)" << std::endl;
	std::cerr << "        graphsensor : draw a graph of measured data of a sensor (which must be selected using -s)" << std::endl;
	std::cerr << "        graphallsensors: draw a graph of all sensors, the 'file=' parameter from -p must be omitted" << std::endl;
	std::cerr << "        gps         : dump gps data" << std::endl;
	std::cerr << "        kml         : emit google maps kml-file, needs 'file=' parameter and a sensor (-s)" << std::endl;
	std::cerr << "        xml         : emit a \"XML spreadsheet\" compatible with at least MS Excel 2007, needs 'file=' parameter" << std::endl;
	std::cerr << "-f x    field seperator, default is tab" << std::endl;
	std::cerr << "-h      this help" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Parameters (-p) for graph(all)sensor(s):" << std::endl;
	std::cerr << "\tparameters must be comma seperated, no spaces" << std::endl;
	std::cerr << "\twidth=x   width of image (default: " << DEFAULT_IMG_WIDTH << ")" << std::endl;
	std::cerr << "\theight=x  height of image (default: " << DEFAULT_IMG_HEIGHT << ")" << std::endl;
	std::cerr << "\tfile=x    PNG file to write to (default: " << DEFAULT_IMG_FILE << ")" << std::endl;
	std::cerr << "\tfont=x    font-file to use (default: " << DEFAULT_IMG_FONT << ")" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Parameters (-p) for dumpsensor:" << std::endl;
	std::cerr << "\tround=x   wether to round down the timestamp (required for gnuplot), true or false" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Parameters (-p) for kml:" << std::endl;
	std::cerr << "\tfile=x    file to store the KML data in" << std::endl;
	std::cerr << "\tintervals=x  in how many pairs to split up the data" << std::endl;
	std::cerr << "\tlinear=x  (yes/no)" << std::endl;
	std::cerr << std::endl;
	std::cerr << "Parameters (-p) for xml spreadsheet:" << std::endl;
	std::cerr << "\tfile=x    file to store the XML data in" << std::endl;
	std::cerr << "\tone_sheet=x  set to \"true\" for all data in one sheet" << std::endl;
	std::cerr << std::endl;
}

int main(int argc, char *argv[])
{
	std::cerr << "O2OO-dumper v" VERSION ", (C) folkert@vanheusden.com" << std::endl << std::endl;

	sqlite3 *db = NULL;

	std::string sensor, parameters;
	std::string cmd, db_file;

	int c;
	while((c = getopt(argc, argv, "b:s:c:p:f:vh")) != -1)
	{
		switch(c)
		{
			case 'b':
				db_file = optarg;
				break;

			case 's':
				sensor = optarg;
				break;

			case 'c':
				cmd = optarg;
				break;

			case 'f':
				field_seperator = optarg;
				break;

			case 'p':
				parameters = optarg;
				break;

			case 'v':
				verbose = true;
				break;

			case 'h':
				help();
				return 0;

			default:
				help();
				return 1;
		}
	}

	if (db_file == "")
		error_exit("No database-file selected to work on");

	if (sqlite3_open(db_file.c_str(), &db))
		error_exit("Problem opening database file %s", db_file.c_str());

	if (cmd == "dumpmetadata")
		dump_metadata(db);
	else if (cmd == "dumpsensor")
	{
		if (sensor == "")
			error_exit("You first need to select a sensor to work on using -s");

		dump_sensor(db, sensor, parameters);
	}
	else if (cmd == "graphsensor")
	{
		if (sensor == "")
			error_exit("You first need to select a sensor to work on using -s");

		graph_sensor(db, sensor, parameters);
	}
	else if (cmd == "graphallsensors")
		graph_all_sensors(db, parameters);
	else if (cmd == "gps")
		emit_gps(db);
	else if (cmd == "kml")
		emit_kml(db, sensor, parameters);
	else if (cmd == "xml")
		emit_xml(db, sensor, parameters);
	else
		error_exit("Command %s is not understood", cmd.c_str());

	if (db)
		sqlite3_close(db);

	return 1;
}

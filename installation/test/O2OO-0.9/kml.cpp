#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sqlite3.h>

#include "error.h"
#include "utils.h"
#include "cluster_data.h"
#include "main.h"
#include "utils-sp.h"
#include "main-dump.h"

void emit_kml(sqlite3 *db, std::string sensor, std::string parameters)
{
	if (sensor.size() == 0)
		error_exit("KML-dump requires a sensor (-s)");

	std::string file;
	int value_intervals = 16;
	bool linear = false;

	std::vector<std::string> pairs = split_string(parameters, ",");
	for(unsigned int pair_index=0; pair_index<pairs.size(); pair_index++)
	{
		std::vector<std::string> pars = split_string(pairs.at(pair_index), "=");
		if (pars.size() != 2)
			error_exit("A parameter must be in 'x=y' format (%s fails)", pairs.at(pair_index).c_str());

		if (pars.at(0) == "file")
			file = pars.at(1);
		else if (pars.at(0) == "linear")
			linear = pars.at(1) == "yes" || pars.at(1) == "true" || pars.at(1) == "on";
		else if (pars.at(0) == "intervals")
		{
			value_intervals = atoi(pars.at(1).c_str());

			if (value_intervals < 2)
				error_exit("Intervals parameter must be 2 or bigger");
		}
		else
		{
			error_exit("Parameter '%s' is not known", pars.at(0).c_str());
		}
	}

	if (file == "")
		error_exit("You need to give a \"file=\" parameter (-p) for KML output");

	// create file
	FILE *fh = fopen(file.c_str(), "wb");
	if (!fh)
		error_exit("Cannot create file %s", file.c_str());

	std::string units;
	double dmi = 0.0, dma = 0.0;
	std::string descr;
	retrieve_metadata(db, sensor, &units, &dmi, &dma, &descr);

	double mi = 0.0, ma = 0.0;
	find_data_bw(db, sensor, &mi, &ma);

	// load location & sensor data
	if (verbose)
		std::cout << "Retrieving data from database...\n";

	std::string query = "SELECT " GPS_TABLE ".ts, longitude, latitude, altitude, value FROM " + sensor + ", " GPS_TABLE " WHERE " + sensor + ".ts = " GPS_TABLE ".ts ORDER BY " GPS_TABLE ".ts ASC";

	std::vector<std::vector<std::string> > results;
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	if (verbose)
		std::cout << "Number of readings with a GPS location (where the GPS had a fix): " << results.size() << std::endl;

	// determine ranges for colors (determine clusters of values)
	if (verbose)
		std::cout << "Clustering data...\n";

	std::vector<double> *splits = NULL;
	if (linear)
	{
		splits = new std::vector<double>();

		double mul = (ma - mi) / double(value_intervals);
		for(int split_index=0; split_index<=value_intervals; split_index++)
			splits -> push_back(mul * double(split_index));
	}
	else
	{
		std::vector<double> values;
		values.push_back(mi);
		values.push_back(ma);
		for(unsigned int row_index=0; row_index<results.size(); row_index++)
			values.push_back(atof(results.at(row_index).at(4).c_str()));

		splits = split_value_list(&values, value_intervals);
	}

	if (verbose)
		std::cout << "Writing KML file...\n";

	fprintf(fh, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(fh, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
	fprintf(fh, "	<Document>\n");
	fprintf(fh, "		<name>%s</name>\n", sensor.c_str());
	fprintf(fh, "		<atom:author>O2OO v" VERSION ", written by folkert@vanheusden.com</atom:author>\n");
	fprintf(fh, "		<atom:link href=\"http://www.vanheusden.com/O2OO/\"/>\n");

	unsigned int last_split_index = splits -> size() - 1;
	for(unsigned int split_index=0; split_index<last_split_index; split_index++)
	{
		if (verbose)
			std::cout << format("%d", last_split_index - split_index) << "   \r";

		double cur_mi = splits -> at(split_index);
		double cur_ma = splits -> at(split_index + 1);

		double cur_half = (cur_mi + cur_ma) / 2.0;
		double progress = (cur_half - mi) / (ma - mi);

		int col_r = (int)std::max(0.0, std::min(255.0, 255.0 * progress));
		int col_g = 0;
		int col_b = 255 - (int)std::max(0.0, std::min(255.0, 255.0 * progress));

		fprintf(fh, "		<Style id=\"style%d\">\n", split_index);
		fprintf(fh, "			<LineStyle>\n");
		fprintf(fh, "				<width>3</width>\n");
		fprintf(fh, "				<color>ff%02x%02x%02x</color>\n", col_b, col_g, col_r);
		fprintf(fh, "			</LineStyle>\n");
		fprintf(fh, "		</Style>\n");
	}

// FIXME multiple datasources:
// <gx:outerColor>ff55ff55</gx:outerColor>
// <gx:outerWidth>0.25</gx:outerWidth>
// (binnen LineStyle)

	for(unsigned int split_index=0; split_index<last_split_index; split_index++)
	{
		if (verbose)
			std::cout << format("%d", last_split_index - split_index) << "   \r";

		double cur_mi = splits -> at(split_index);
		double cur_ma = splits -> at(split_index + 1);

		fprintf(fh, "<Folder id=\"folder%d\">\n", split_index);
		fprintf(fh, "	<open>0</open>\n");
		fprintf(fh, "	<name>%f ... %f %s</name>\n", cur_mi, cur_ma, units.c_str());

		std::vector<std::string> *last = NULL;
		int prev_data_index = -2;
		bool in_range = false;
		for(int data_index=0; data_index<results.size(); data_index++)
		{
			std::vector<std::string> *row = &results.at(data_index);

			if (in_range)
			{
				in_range = false;
				last = row;
			}

			double value = atof(row -> at(4).c_str());

			if (value < cur_mi || value >= cur_ma)
				continue;

			in_range = true;

			if (prev_data_index != data_index -1)
			{
				if (prev_data_index != -2)
				{
					fprintf(fh, "					%s,%s,%s\n", last -> at(1).c_str(), last -> at(2).c_str(), last -> at(3).c_str());

					fprintf(fh, "				</coordinates>\n");
					fprintf(fh, "			</LineString>\n");
					fprintf(fh, "		</Placemark>\n");
				}

				fprintf(fh, "		<Placemark id=\"pm%d-%d\">\n", split_index, data_index);
				fprintf(fh, "			<name>%f ... %f %s</name>\n", cur_mi, cur_ma, units.c_str());
				fprintf(fh, "			<styleUrl>#style%d</styleUrl>\n", split_index);
				fprintf(fh, "			<LineString>\n");
				fprintf(fh, "				<coordinates>\n");
			}

			fprintf(fh, "					%s,%s,%s\n", row -> at(1).c_str(), row -> at(2).c_str(), row -> at(3).c_str());

			prev_data_index = data_index;
		}

		if (prev_data_index != -2)
		{
			fprintf(fh, "					%s,%s,%s\n", last -> at(1).c_str(), last -> at(2).c_str(), last -> at(3).c_str());
			fprintf(fh, "				</coordinates>\n");
			fprintf(fh, "			</LineString>\n");
			fprintf(fh, "		</Placemark>\n");
		}

		fprintf(fh, "</Folder>\n");
	}

	fprintf(fh, "	</Document>\n");
	fprintf(fh, "</kml>\n");

	fclose(fh);

	delete splits;
}

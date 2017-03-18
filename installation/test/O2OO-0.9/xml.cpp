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
#include "main-dump.h"

std::string gps_location = GPS_TABLE;

void emit_multiple_sheets(sqlite3 *db, FILE *fh)
{
	std::string query = "SELECT table_name FROM " SENSOR_META_DATA " ORDER BY table_name";

	std::vector<std::vector<std::string> > results_meta;
	char *error = NULL;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results_meta, &error))
		error_exit("DB error: %s", error);

	for(unsigned int index=0; index<results_meta.size(); index++)
	{
		std::vector<std::string> *row = &results_meta.at(index);
		std::string table_name = row -> at(0);

		if (verbose)
			std::cout << " - emitting data for " << table_name << std::endl;

		std::string units, descr;
		double d_mi, d_ma;
		retrieve_metadata(db, table_name, &units, &d_mi, &d_ma, &descr);

		fprintf(fh, "<Worksheet ss:Name=\"%d\">\n", index);
		fprintf(fh, "<Table>\n");
		fprintf(fh, "<Row>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">sensor name:</Data></Cell>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">%s</Data></Cell>\n", table_name.c_str());
		fprintf(fh, "</Row>\n");
		fprintf(fh, "<Row>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">description:</Data></Cell>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">%s</Data></Cell>\n", descr.c_str());
		fprintf(fh, "</Row>\n");
		fprintf(fh, "<Row>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">units:</Data></Cell>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">%s</Data></Cell>\n", units.c_str());
		fprintf(fh, "</Row>\n");
		fprintf(fh, "<Row>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">minimum value:</Data></Cell>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"Number\">%f</Data></Cell>\n", d_mi);
		fprintf(fh, "</Row>\n");
		fprintf(fh, "<Row>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">maximum value:</Data></Cell>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"Number\">%f</Data></Cell>\n", d_ma);
		fprintf(fh, "</Row>\n");

		std::string query = "SELECT ts, value FROM " + table_name + " ORDER BY nr ASC";

		std::vector<std::vector<std::string> > results;
		char *error = NULL;

		if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
			error_exit("DB error: %s", error);

		for(unsigned int index=0; index<results.size(); index++)
		{
			fprintf(fh, "<Row>\n");
			std::string date_str = time_to_str(atof(results.at(index).at(0).c_str()));
			fprintf(fh, "<Cell ss:StyleID=\"sMD\"><Data ss:Type=\"DateTime\">%s</Data></Cell>\n", date_str.c_str());
			fprintf(fh, "<Cell><Data ss:Type=\"Number\">%s</Data></Cell>\n", results.at(index).at(1).c_str());
			fprintf(fh, "</Row>\n");
		}

		fprintf(fh, "</Table>\n");
		fprintf(fh, "</Worksheet>\n");
	}
}

void emit_one_sheet(sqlite3 *db, FILE *fh)
{
	std::string meta_query = "SELECT table_name FROM " SENSOR_META_DATA " ORDER BY table_name";

	std::vector<std::vector<std::string> > results_meta;
	char *error = NULL;

	if (sqlite3_exec(db, meta_query.c_str(), sl_callback, (void *)&results_meta, &error))
		error_exit("DB error: %s", error);

	std::string fields;
	std::string from;
	std::string where;
	bool first = true;
	std::string nr_table;

	for(unsigned int index=0; index<results_meta.size(); index++)
	{
		std::string cur_table = results_meta.at(index).at(0);

		if (first)
		{
			first = false;
			fields = cur_table + ".ts, ";
			nr_table = cur_table;
		}
		else
			fields += ", ";

		fields += cur_table + ".value";

		if (from != "")
			from += ", ";

		from += cur_table;

		if (where != "")
			where += " AND ";

		if (cur_table != nr_table)
			where += cur_table + ".ts=" + nr_table + ".ts";
	}

	if (check_table_exists(db, gps_location))
	{
		fields += ", longitude, latitude, altitude";
		from += ", " + gps_location;
		where += " AND " + gps_location + ".ts=" + nr_table + ".ts";
	}

	std::string query = "SELECT " + fields + " FROM " + from + " WHERE " + where + " ORDER BY " + nr_table + ".nr";

	std::vector<std::vector<std::string> > results;

	if (sqlite3_exec(db, query.c_str(), sl_callback, (void *)&results, &error))
		error_exit("DB error: %s", error);

	fprintf(fh, "<Worksheet ss:Name=\"OBDII data\">\n");
	fprintf(fh, "<Table>\n");

	fprintf(fh, "<Row>\n");
	fprintf(fh, "<Cell><Data ss:Type=\"String\">timestamp</Data></Cell>\n");
	for(unsigned int index=0; index<results_meta.size(); index++)
		fprintf(fh, "<Cell><Data ss:Type=\"String\">%s</Data></Cell>\n", results_meta.at(index).at(0).c_str());
	if (check_table_exists(db, gps_location))
	{
		fprintf(fh, "<Cell><Data ss:Type=\"String\">longitude</Data></Cell>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">latitude</Data></Cell>\n");
		fprintf(fh, "<Cell><Data ss:Type=\"String\">altitude</Data></Cell>\n");
	}
	fprintf(fh, "</Row>\n");

	for(unsigned int index=0; index<results.size(); index++)
	{
		std::vector<std::string> *row = &results.at(index);

		fprintf(fh, "<Row>\n");

		std::string date_str = time_to_str(atof(row -> at(0).c_str()));
		fprintf(fh, "<Cell ss:StyleID=\"sMD\"><Data ss:Type=\"DateTime\">%s</Data></Cell>\n", date_str.c_str());

		for(unsigned int field=1; field<row -> size(); field++)
			fprintf(fh, "<Cell><Data ss:Type=\"Number\">%s</Data></Cell>\n", row -> at(field).c_str());

		fprintf(fh, "</Row>\n");
	}

	fprintf(fh, "</Table>\n");
	fprintf(fh, "</Worksheet>\n");
}

void emit_xml(sqlite3 *db, std::string sensor, std::string parameters)
{
	std::string file;

	bool one_sheet = false;

	std::vector<std::string> pairs = split_string(parameters, ",");
	for(unsigned int pair_index=0; pair_index<pairs.size(); pair_index++)
	{
		std::vector<std::string> pars = split_string(pairs.at(pair_index), "=");
		if (pars.size() != 2)
			error_exit("A parameter must be in 'x=y' format (%s fails)", pairs.at(pair_index).c_str());

		if (pars.at(0) == "file")
			file = pars.at(1);
		else if (pars.at(0) == "one_sheet")
			one_sheet = pars.at(1) == "yes" || pars.at(1) == "true";
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

	if (verbose)
		std::cout << "Writing XML spreadsheet file..." << std::endl;

	fprintf(fh, "<?xml version=\"1.0\"?>\n");
	fprintf(fh, "<?mso-application progid=\"Excel.Sheet\"?>\n");
	fprintf(fh, "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\n");
	fprintf(fh, " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"\n");
	fprintf(fh, " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\n");
	fprintf(fh, " xmlns:html=\"http://www.w3.org/TR/REC-html40\">\n");
	fprintf(fh, "<DocumentProperties xmlns=\"urn:schemas-microsoft-com:office:office\">\n");
	fprintf(fh, "<Author>O2OO</Author>\n");
	fprintf(fh, "<Company>folkert@vanheusden.com</Company>\n");
	fprintf(fh, "<Version>" VERSION "</Version>\n");
	fprintf(fh, "</DocumentProperties>\n");
	fprintf(fh, "<Styles>\n");
	fprintf(fh, "<Style ss:ID=\"sMD\">\n");
	fprintf(fh, "<NumberFormat ss:Format=\"Long Time\"/>\n");
	fprintf(fh, "</Style>\n");
	fprintf(fh, "</Styles>\n");

	if (one_sheet)
		emit_one_sheet(db, fh);
	else
		emit_multiple_sheets(db, fh);

	fprintf(fh, "</Workbook>\n");

	fclose(fh);
}

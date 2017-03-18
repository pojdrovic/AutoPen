// SVN: $Revision: 475 $
#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <pwd.h>
#include <ncurses.h>
#include <sqlite3.h>

#include "error.h"
#include "terminal.h"
#include "terminal_console.h"
#include "terminal_ncurses.h"
#include "utils.h"
#include "byte_array.h"
#include "serial_io.h"
#include "serial_io_tcpip.h"
#include "gps.h"
#include "gpsd.h"
#include "IO_Device.h"
#include "IO_ELM327.h"
#include "IO_Test.h"
#include "sensors/sensor.h"
#include "sensors/io_sensor.h"
#include "OBD2.h"
#include "main.h"

// full_speed: some CAN-bus's/ODBs don't like probing more than once per 100ms
bool full_speed = false;
int verbose = 0;
io_mode_t mode = MODE_ELM327;
commit_mode_t cm_mode = CM_BATCH;
double cm_interval = 5.0;
display_mode_t d_mode = DISP_DASHBOARD;
std::string symbol_clock = "clock";
std::string symbol_gps_loc = "gps_loc";
std::string symbol_gps_fix = "gps_fix";
std::string symbol_gps_db = "gps_db";
std::string symbol_gps_n = "gps_n";
std::string symbol_n_stored_locs = "n_stored_locs";
std::string symbol_l3_errors = "l3_errors"; // ELM327 level
std::string symbol_l7_errors = "l7_errors"; // OBD2 level

terminal *t = NULL;

std::string table_gps = GPS_TABLE;
std::string table_sensor_meta_data = SENSOR_META_DATA;
std::string table_sensor_prefix = SENSOR_TABLE_PREFIX;
std::string table_global_info = META_DATA;

volatile bool terminate = false;

void sig_handler(int sig)
{
	terminate = true;
}

void create_sensor_table(sqlite3 *db, std::string table, std::string units, double mi, double ma, std::string descr)
{
	if (!check_table_exists(db, table))
	{
		std::string query = std::string("CREATE TABLE ") + table + "(value DOUBLE NOT NULL, ts DOUBLE NOT NULL, nr INTEGER PRIMARY KEY ASC)";

		t -> emit(LL_DEBUG_ALL, query);

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}

	// insert record into meta data database
	if (get_query_value(db, "SELECT COUNT(*) AS n FROM " + table_sensor_meta_data + " WHERE table_name='" + table +"'") == 0)
	{
		std::string query_meta = std::string("INSERT INTO ") + table_sensor_meta_data + "(table_name, units, min, max, descr) VALUES('" + table + "', '" + units + "', " + format("%f", mi) + ", " + format("%f", ma) + ", '" + descr + "')";

		t -> emit(LL_DEBUG_ALL, query_meta);

		char *error = NULL;
		if (sqlite3_exec(db, query_meta.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}
}

void create_table_sensor_meta_data(sqlite3 *db)
{
	if (!check_table_exists(db, table_sensor_meta_data))
	{
		std::string query = "CREATE TABLE " + table_sensor_meta_data + "(table_name TEXT NOT NULL, units TEXT NOT NULL, min DOUBLE NOT NULL, max DOUBLE NOT NULL, descr TEXT NOT NULL, PRIMARY KEY(table_name))";

		t -> emit(LL_DEBUG_ALL, query);

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}
}

void create_table_global_info(sqlite3 *db)
{
	if (!check_table_exists(db, table_global_info))
	{
		std::string query = "CREATE TABLE " + table_global_info + "(key TEXT NOT NULL, value TEXT NOT NULL, PRIMARY KEY(key))";

		t -> emit(LL_DEBUG_ALL, query);

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}
}

void process_sensor_value(double ts, sqlite3 *db, sensor *s, double value)
{
	std::string sensor_id = s -> get_symbol();

	if (db)
	{
		std::string query = std::string("INSERT INTO ") + table_sensor_prefix + sensor_id + "(value, ts) VALUES(" + format("%8E", value) + ", " + format("%f", ts) + ")";

		t -> emit(LL_DEBUG_ALL, query);

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}

	std::string output = format((s -> get_description() + ": |" + s -> get_screen_formatting()).c_str(), value);

	t -> put(sensor_id, output);
}

void store_gps_data(sqlite3 *db, bool has_fix, double ts, double lo, double la, double al, int n_sats, double ground_speed, std::string nmea_coords)
{
	if (db && has_fix)
	{
		std::string query = std::string("INSERT INTO ") + table_gps + "(ts, longitude, latitude, altitude, n_sats, ground_speed) VALUES(" + format("%f, %8E, %8E, %8E, %d, %8E",  ts, lo, la, al, n_sats, ground_speed) + ")";

		t -> emit(LL_DEBUG_ALL, query);

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}

	t -> put(symbol_gps_loc, "GPS: |" + nmea_coords);
}

void insert_into_global_info(sqlite3 *db, std::string key, std::string value, bool update_if_exists = false)
{
	if (get_query_value(db, "SELECT COUNT(*) FROM " + table_global_info + " WHERE key='" + key + "'") == 0)
	{
		std::string query = "INSERT INTO " + table_global_info + "(key, value) VALUES('" + key + "', '" + value + "')";

		t -> emit(LL_DEBUG_ALL, query);

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}
	else if (update_if_exists)
	{
		std::string query = "UPDATE " + table_global_info + " SET value='" + value + "' WHERE key='" + key + "'";

		t -> emit(LL_DEBUG_ALL, query);

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}
}

void fill_global_info(sqlite3 *db, std::string db_file, std::string io_mode, std::string gps_mode, std::string dev_info, std::vector<std::string> *gm_strs, OBD2 *o)
{
	char buffer[4096];

	std::map<std::string, sensor *> *sensors = o -> get_detected_sensors();
	std::map<std::string, sensor *>::iterator it = sensors -> begin();
	for(; it != sensors -> end(); it++)
	{
		if (!it -> second -> is_meta())
			continue;

		std::string value;
		if (it -> second -> get_value(t, o, &value))
			insert_into_global_info(db, it -> second -> get_description(), value);
	}

	int fuel_type = -1;
	if (o -> get_is_supported(0x01, 0x51) && o -> get_pid_8b(0x51, &fuel_type))
		insert_into_global_info(db, "fuel type", format("%02x", fuel_type));

	std::string VIN;
	if (o -> get_VIN(&VIN))
		insert_into_global_info(db, "VIN", VIN);

	// std::string calibration_ID;
	// if (o -> get_calibration_ID(&calibration_ID))
		// insert_into_global_info(db, "calibration id", calibration_ID);

	insert_into_global_info(db, "database file", db_file);
	insert_into_global_info(db, "I/O mode", io_mode);
	insert_into_global_info(db, "CAN device type", dev_info);
	if (gps_mode != "")
		insert_into_global_info(db, "GPS", gps_mode);

	if (gethostname(buffer, sizeof buffer) == -1)
		error_exit("gethostname() failed");
	insert_into_global_info(db, "host name", buffer);

	uid_t user = getuid();
	struct passwd *pw = NULL;
	for(;;)
	{
		pw = getpwent();
		if (pw == NULL)
			break;
		if (user == pw -> pw_uid)
			break;
	}
	insert_into_global_info(db, "user", pw ? pw -> pw_name : "?");

	insert_into_global_info(db, "date/time start", get_time_str());

	for(unsigned int index=0; index<gm_strs -> size(); index++)
	{
		std::vector<std::string> parts = split_string(gm_strs -> at(index), "|");
		if (parts.size() != 2)
			error_exit("-M parameter '%s' should consist of 2 elements seperated with '|'", gm_strs -> at(index).c_str());

		insert_into_global_info(db, parts.at(0), parts.at(1));
	}
}

void create_gps_table(sqlite3 *db)
{
	if (!check_table_exists(db, table_gps))
	{
		std::string query = "CREATE TABLE " + table_gps + "(ts DOUBLE NOT NULL, longitude DOUBLE NOT NULL, latitude DOUBLE NOT NULL, altitude DOUBLE NOT NULL, n_sats INTEGER NOT NULL, ground_speed DOUBLE NOT NULL, nr INTEGER PRIMARY KEY ASC)";

		t -> emit(LL_DEBUG_ALL, query);

		char *error = NULL;
		if (sqlite3_exec(db, query.c_str(), NULL, NULL, &error))
			error_exit("DB error: %s", error);
	}
}

void register_sensor(sqlite3 *db, terminal *t, sensor *s)
{
	if (verbose)
	{
		t -> emit(LL_DEBUG_SOME, "will monitor sensor: " + s -> get_symbol());
		t -> update_terminal();
	}

	// determine length of string: needed to fit all strings in the top-
	// window
	std::string screen_text = s -> get_description() + ": " + s -> get_screen_formatting();

	char buffer[4096];
	int width = snprintf(buffer, sizeof buffer, screen_text.c_str(), -1.1);

	t -> allocate_space(width, s -> get_color(), s -> get_symbol());

	if (db)
	{
		double mi = -1.0, ma = 1.0;
		s -> get_range(&mi, &ma);
		create_sensor_table(db, table_sensor_prefix + s -> get_symbol(), s -> get_unit(), mi, ma, s -> get_description());
	}
}

void register_sensors(sqlite3 *db, OBD2 *o, terminal *t)
{
	if (db)
	{
		start_transaction(db);

		create_table_sensor_meta_data(db);
	}

	std::map<std::string, sensor *> *sensors =  o -> get_detected_sensors();
	std::map<std::string, sensor *>::iterator it = sensors -> begin();

	for(; it != sensors -> end(); it++)
	{
		if (!it -> second -> is_meta())
			register_sensor(db, t, it -> second);
	}

	if (db)
		commit_transaction(db);
}

void filter_sensors(OBD2 *o, std::string ignore_list)
{
	std::vector<std::string> sensors = split_string(ignore_list, ",");

	for(unsigned int index=0; index<sensors.size(); index++)
	{
		if (!o -> ignore_sensor(sensors.at(index)))
			error_exit("Sensor %s not found or was not supported anyway", sensors.at(index).c_str());
	}
}

void draw_clock(double start_ts)
{
	time_t ts = time(NULL);
	struct tm *tm = localtime(&ts);

	std::string out = format("%02d:%02d:%02d |(%3.0fmin)", tm -> tm_hour, tm -> tm_min, tm -> tm_sec, (get_ts() - start_ts) / 60.0);

	t -> put(symbol_clock, out);
}

// this check because a new invocation can:
// - be on a different vehicle with different PIDs
// - be a test run in different environmental (weather) conditions
void check_db_file_not_exists(std::string db_file)
{
	struct stat st;

	if (stat(db_file.c_str(), &st) == 0)
		error_exit("The database file you selected (%s) already exists.\nStart this program with the -a switch if you're sure you want to add new data to this file.", db_file.c_str());
}

void help(void)
{
	std::cerr << "-d x    serial port (connected to ELM327) to read from (default: " DEFAULT_SERIAL_PORT ")" << std::endl;
	std::cerr << "        if this port does not start with \"/dev\" then it is assumed this parameter is a host (ip address) and port number to connect to (for those ELM327 over WIFI dongles) (e.g. \"192.168.0.1 1234\")" << std::endl;
	std::cerr << "-b x    database (SQLite) to write to" << std::endl;
	std::cerr << "-a      add to an existing(!) database-file" << std::endl;
	std::cerr << "-m x    mode (\"elm327\" (default) or \"test\" (which emulates a car))" << std::endl;
	std::cerr << "-i x    list of sensors to ignore, comma seperated (e.g. sensor_engine_rpm,sensor_engine_load)" << std::endl;
	std::cerr << "-o x    output: \"simple\" for logging only, \"dashboard\" for dashboard-like mode" << std::endl;
	std::cerr << "-g x    device on which an NMEA emitting GPS is connected (use \"fake\" for an emulated gps, enter a port number for gpsd)" << std::endl;
	std::cerr << "-G x    bit rate for communicating with the GPS device. usually this is 4800 (which is the default, use -1 to not change the serial port settings at all)" << std::endl;
	std::cerr << "-M x|y  add a key/value pair to the meta-data table. will be used in reporting. e.g. \"customer|john doe\". you can use multiple -M invocations" << std::endl;
	// std::cerr << "-I      retrieve related data from internet (requires an internet connection, data retrieved is weather conditions at the location we're measuring (requires GPS))" << std::endl;
	std::cerr << "-l x    write logging to file x" << std::endl;
	std::cerr << "-c      enable colors (for \"-o dashboard\")" << std::endl;
	std::cerr << "-v      increase verbosity" << std::endl;
	std::cerr << "-f      enable full speed mode. please note that this only works with vehicles that implement APR2002" << std::endl;
	std::cerr << "-h      this help" << std::endl;
}

int main(int argc, char *argv[])
{
	std::cout << "O2OO-collector v" VERSION ", (C) folkert@vanheusden.com" << std::endl;

	gps *g = NULL;

	std::string serial_port = DEFAULT_SERIAL_PORT;
	std::string db_file;
	std::string ignore_list;
	std::string gps_dev;
	std::string log_file;
	int gpsd_port = -1;
	bool colors = false;
	int gps_bps = 4800;
	bool add = false;
	bool use_internet = false;
	std::vector<std::string> gm_strs;

	int c;
	while((c = getopt(argc, argv, "d:b:am:i:o:g:G:M:I:l:cvfh")) != -1)
	{
		switch(c)
		{
			case 'd':
				serial_port = optarg;
				break;

			case 'b':
				db_file = optarg;
				break;

			case 'a':
				add = true;
				break;

			case 'm':
				if (strcasecmp(optarg, "elm327") == 0)
					mode = MODE_ELM327;
				else if (strcasecmp(optarg, "test") == 0)
					mode = MODE_TEST;
				else
					error_exit("-m: %s is not known", optarg);
				break;

			case 'i':
				ignore_list = optarg;
				break;

			case 'o':
				if (strcasecmp(optarg, "simple") == 0)
					d_mode = DISP_SIMPLE;
				else if (strcasecmp(optarg, "dashboard") == 0)
					d_mode = DISP_DASHBOARD;
				else
					error_exit("-d: %s is not known", optarg);
				break;

			case 'M':
				gm_strs.push_back(optarg);
				break;

			case 'g':
				if (atoi(optarg) != 0)
					gpsd_port = atoi(optarg);
				else
					gps_dev = optarg;
				break;

			case 'G':
				gps_bps = atoi(optarg);
				break;

			case 'I':
				use_internet = true;
				break;

			case 'l':
				log_file = optarg;
				break;

			case 'c':
				colors = true;
				break;

			case 'v':
				verbose++;
				break;

			case 'f':
				full_speed = true;
				break;

			case 'h':
				help();
				return 0;

			default:
				help();
				return 1;
		}
	}

	if (d_mode == DISP_SIMPLE)
		t = new terminal_console(verbose, colors, log_file);
	else if (d_mode == DISP_DASHBOARD)
		t = new terminal_ncurses(verbose, colors, log_file);

	t -> emit(LL_INFO, "O2OO-collector v" VERSION ", (C) folkert@vanheusden.com");
	t -> emit(LL_INFO, std::string("SQLite version: ") + sqlite3_libversion());
	t -> emit(LL_INFO, "SVN revision of main.cpp: $Revision: 475 $");
	t -> emit(LL_INFO, "");
	t -> emit(LL_INFO, "Steps:");
	t -> emit(LL_INFO, "\t1. switch off vehicle");
	t -> emit(LL_INFO, "\t2. connect ELM327 module");
	t -> emit(LL_INFO, "\t3. start vehicle AND engine");
	t -> emit(LL_INFO, "\t4. start this program");
	t -> update_terminal();
	t -> emit(LL_INFO, "");
	t -> emit(LL_INFO, std::string("Serial port: ") + serial_port);
	t -> emit(LL_INFO, std::string("DB file    : ") + db_file);
	if (db_file == "")
		t -> emit(LL_INFO, "NO DATABASE FILE SELECTED!");
	t -> emit(LL_INFO, std::string("Full speed : ") + (full_speed ? "yes" : "no"));
	if (gps_dev != "")
		t -> emit(LL_INFO, std::string("GPS device : ") + gps_dev);
	if (gpsd_port != -1)
		t -> emit(LL_INFO, format("GPSd port  : %d", gpsd_port));
	t -> update_terminal();
	t -> emit(LL_INFO, "");

	signal(SIGHUP , SIG_IGN);
	signal(SIGTERM, sig_handler);
	signal(SIGINT , sig_handler);
	signal(SIGQUIT, sig_handler);
	signal(SIGPIPE, SIG_IGN);

	if (db_file != "" && add == false)
		check_db_file_not_exists(db_file);

	sqlite3 *db = NULL;
	if (db_file != "")
	{
		if (sqlite3_open(db_file.c_str(), &db))
			error_exit("Problem creating database file %s", db_file.c_str());
	}

	t -> allocate_space(17, C_WHITE, symbol_clock);

	serial_io *gps_sio = NULL;
	if (gps_dev != "")
	{
		t -> emit(LL_INFO, "Opening connection to GPS...");
		t -> update_terminal();

		if (gps_dev != "fake")
			gps_sio = new serial_io(gps_dev, gps_bps);
		g = new gps(gps_sio, t);
	}

	if (gpsd_port != -1)
	{
		t -> emit(LL_INFO, "Opening connection to GPS...");
		t -> update_terminal();

		g = new gpsd("localhost", gpsd_port, t);
	}

	if (g != NULL)
	{
		t -> allocate_space(34, C_YELLOW, symbol_gps_loc);
		t -> allocate_space(7, C_GREEN, symbol_gps_fix);
		t -> allocate_space(16, C_CYAN, symbol_gps_n);
		t -> allocate_space(12, C_MAGENTA, symbol_gps_db);

		if (db != NULL)
			create_gps_table(db);
	}

	t -> allocate_space(14, C_MAGENTA, symbol_n_stored_locs);

	t -> allocate_space(15, C_RED, symbol_l3_errors);
	t -> allocate_space(15, C_YELLOW, symbol_l7_errors);

	serial_io *sio = NULL;
	if (mode != MODE_TEST)
	{
		if (serial_port.substr(0, 4) == "/dev")
		{
			t -> emit(LL_INFO, "Opening serial port...");
			t -> update_terminal();

			sio = new serial_io(serial_port, 9600);
		}
		else
		{
			std::string host = "192.168.0.10";
			int port = 35000;
			std::vector<std::string> pars = split_string(serial_port, " ");
			if (pars.size() == 2)
				port = atoi(pars.at(1).c_str());
			host = pars.at(0);

			t -> emit(LL_INFO, "Connecting to " + host + " " + format("%d", port));
			t -> update_terminal();

			sio = new serial_io_tcpip(host, port);
		}
	}

	IO_Device *io = NULL;

	t -> emit(LL_INFO, "Handshaking with I/O port...");
	t -> update_terminal();

	if (mode == MODE_ELM327)
		io = new IO_ELM327(sio, t, full_speed);
	else if (mode == MODE_TEST)
		io = new IO_Test(t, full_speed);
	else
		error_exit("Internal error (io-mode %d not known)", mode);

	t -> emit(LL_DEBUG_SOME, "Retrieving list of sensors...");
	t -> update_terminal();

	OBD2 *o = new OBD2(io, t);

	t -> emit(LL_INFO, "Sensors:");
	t -> update_terminal();

	o -> list_supported_pids();
	t -> update_terminal();

	ignore_list = trim(ignore_list);
	if (ignore_list != "")
	{
		t -> emit(LL_DEBUG_SOME, "Removing filtered sensors...");

		filter_sensors(o, ignore_list);
	}

	t -> emit(LL_DEBUG_SOME, "Initializing database...");
	t -> update_terminal();

	register_sensors(db, o, t);

	if (cm_mode == CM_INTERVAL && db)
		start_transaction(db);

	double last_commit_ts = get_ts();

	t -> recreate_terminal();

	std::string dummy, gps_type;
	if (gps_dev != "")
	{
		if (gps_dev == "fake")
			gps_type = dummy = "fake GPS";
		else
			gps_type = dummy = "GPS";
	}
	std::string io_type;
	if (mode == MODE_ELM327)
	{
		io_type = "EM327";
		dummy += " | " + io_type;
	}
	else if (mode == MODE_TEST)
	{
		io_type = "fake car";
		dummy += " | " + io_type;
	}
	if (full_speed)
	{
		io_type += " (full speed)";
		dummy += " (full speed)";
	}
	if (db_file == "")
		dummy += " | NO DB FILE!";
	else
		dummy += " | DB file: " + db_file;
	t -> set_status("Press ctrl + c to terminate | " + dummy);

	create_table_global_info(db);
	fill_global_info(db, db_file, io_type, gps_type, io -> get_source_type(), &gm_strs, o);

	t -> emit(LL_INFO, "Starting measurements...");
	t -> update_terminal();

	int n = 0;
	double start_ts = get_ts();

	bool had_fix = false;
	for(;!terminate;)
	{
		t -> emit(LL_DEBUG_SOME, get_time_str());

		if (cm_mode == CM_BATCH && db)
			start_transaction(db);

		draw_clock(start_ts);

		// use the same timestamp for the gps and all sensors so that things
		// can be correlated more easy
		double use_ts = get_ts();

		double ts, lo, la, al;
		if (g)
		{
			bool fix = g -> get_coordinates(&ts, &lo, &la, &al);
			if (fix)
			{
				t -> put(symbol_gps_fix, "|!GPS FIX");

				if (!had_fix)
				{
					had_fix = true;

					if (use_internet)
					{
						// * wunderground, requires api key
						// grab http://api.wunderground.com/auto/wui/geo/GeoLookupXML/index.xml?query=37.76834106,-122.39418793 to global db
						// lat,lon
					}
				}
			}
			else
			{
				t -> put(symbol_gps_fix, "       ");
			}

			int n_sats = g -> get_n_satellites_in_view();
			t -> put(symbol_gps_n, format("# sattelites: |%2d", n_sats));

			double sig_db = g -> get_signal_strength();
			t -> put(symbol_gps_db, format("GPS DB: |%4.1f", sig_db));

			double ground_speed = g -> get_ground_speed();

			std::string nmea_coords;
			g -> get_nmea_coordinates(&nmea_coords);

			store_gps_data(db, fix, use_ts, lo, la, al, n_sats, ground_speed, nmea_coords);
		}

		std::map<std::string, sensor *> *sensors =  o -> get_detected_sensors();
		std::map<std::string, sensor *>::iterator it = sensors -> begin();

		bool stored_one = false;
		double sensor_start_ts = use_ts;
		for(; it != sensors -> end() && !terminate; it++)
		{
			double value = -1.0;

			if (it -> second -> is_meta())
				continue;

			if (it -> second -> get_value(t, o, &value))
			{
				process_sensor_value(use_ts, db, it -> second, value);
				stored_one = true;
			}

			double now_ts = get_ts();
			if (now_ts - sensor_start_ts >= 1.0/3.0)
			{
				t -> update_terminal();

				sensor_start_ts = now_ts;
			}
		}

		if (cm_mode == CM_BATCH && db)
			commit_transaction(db);
		else if (cm_mode == CM_INTERVAL && db)
		{
			double now_ts = get_ts();

			if (now_ts - last_commit_ts >= cm_interval)
			{
				t -> emit(LL_DEBUG_SOME, "Commit transaction in database");

				commit_transaction(db);
				start_transaction(db);

				last_commit_ts = now_ts;
			}
		}

		if (stored_one)
			n++;

		t -> put(symbol_n_stored_locs, format("stored: |!%06d", n));

		int dummy_val;
		int n_l3_fail = 0;
		io -> get_io_stats(&dummy_val, &n_l3_fail);
		t -> put(symbol_l3_errors, format("L3 errors: |!%04d", n_l3_fail));

		int n_l7_fail = 0;
		double dummy_val_d;
		o -> get_io_stats(&dummy_val, &n_l7_fail, &dummy_val_d);
		t -> put(symbol_l7_errors, format("L7 errors: |!%04d", n_l7_fail));

		t -> update_terminal();

		t -> emit(LL_DEBUG_ALL, "");
	}

	double end_ts = get_ts();

	insert_into_global_info(db, "date/time end", get_time_str(), true);

	insert_into_global_info(db, "time per batch", format("%fs", (end_ts - start_ts) / double(n)), true);

	if (cm_mode == CM_INTERVAL && db)
	{
		t -> emit(LL_DEBUG_SOME, "Commit transaction in database");
		t -> update_terminal();

		commit_transaction(db);
	}

	t -> emit(LL_INFO, format("%d sensor-sweeps were performed (%d sensors), %f per second", n, o -> get_detected_sensors() -> size(), double(n) / (end_ts - start_ts)));
	t -> update_terminal();

	int n_l3_fail = 0, n_l3_ok = 0, n_l7_fail = 0, n_l7_ok = 0;
	io -> get_io_stats(&n_l3_ok, &n_l3_fail);
	double avg_resp_time = -1.0;
	o -> get_io_stats(&n_l7_ok, &n_l7_fail, &avg_resp_time);
	t -> emit((n_l3_fail > 0 || n_l7_fail > 0) ? LL_ERROR : LL_INFO, format("L3 errors: %d (%d ok), L7 errors: %d (%d ok)", n_l3_fail, n_l3_ok, n_l7_fail, n_l7_ok));

	insert_into_global_info(db, "average response time", format("%fs", avg_resp_time));
	insert_into_global_info(db, "L3 errors", format("%d", n_l3_fail));
	insert_into_global_info(db, "L3 OK", format("%d", n_l3_ok));
	insert_into_global_info(db, "L7 errors", format("%d", n_l7_fail));
	insert_into_global_info(db, "L7 OK", format("%d", n_l7_ok));
	insert_into_global_info(db, "number of sensors", format("%d", o -> get_detected_sensors() -> size()));

	if (db)
		sqlite3_close(db);

	delete o;
	delete io;
	delete sio;
	delete g;
	delete gps_sio;

	t -> emit(LL_INFO, "Finished");
	t -> update_terminal();

	delete t;

	return 0;
}

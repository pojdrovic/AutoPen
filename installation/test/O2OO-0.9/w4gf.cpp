// SVN: $Revision: 442 $
#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "error.h"
#include "terminal.h"
#include "terminal_console.h"
#include "serial_io.h"
#include "gps.h"

int verbose = 0;
terminal *t = NULL;

void help(void)
{
	std::cerr << "-g x    device on which an NMEA emitting GPS is connected" << std::endl;
	std::cerr << "-G x    bit rate for communicating with the GPS device. usually this is 4800 (which is the default, use -1 to not change the serial port settings at all)" << std::endl;
	std::cerr << "-m      do not exit when fix was detected (monitor mode)" << std::endl;
	std::cerr << "-v      increase verbosity" << std::endl;
	std::cerr << "-h      this help" << std::endl;
}

int main(int argc, char *argv[])
{
	std::cout << "O2OO-w4gf v" VERSION ", (C) folkert@vanheusden.com" << std::endl;

	gps *g = NULL;

	std::string gps_dev;
	int gps_bps = 4800;
	bool fix_exit = true;

	int c;
	while((c = getopt(argc, argv, "g:G:mvh")) != -1)
	{
		switch(c)
		{
			case 'g':
				gps_dev = optarg;
				break;

			case 'G':
				gps_bps = atoi(optarg);
				break;

			case 'm':
				fix_exit = false;
				break;

			case 'v':
				verbose++;
				break;

			case 'h':
				help();
				return 0;

			default:
				help();
				return 1;
		}
	}

	t = new terminal_console(verbose, true, "");

	if (gps_dev == "")
		error_exit("Please select a GPS device");

	serial_io *gps_sio = new serial_io(gps_dev, gps_bps);
	g = new gps(gps_sio, t);

	for(;;)
	{
		int n_sats = g -> get_n_satellites_in_view();

		double sig_db = g -> get_signal_strength();

		double ground_speed = g -> get_ground_speed();

		std::string nmea_coords;
		bool fix = g -> get_nmea_coordinates(&nmea_coords);

		printf("coordinates: %s\tground speed: %6.2f\tsignal db: %4f\tnumber of satellites: %2d",
			nmea_coords.c_str(), ground_speed, sig_db, n_sats);

		if (fix)
		{
			printf("\tGPS has fix!");

			if (fix_exit)
				break;
		}

		printf("\n");

		sleep(1);
	}

	delete g;
	delete gps_sio;
	delete t;

	return 0;
}

#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <gps.h>

#include "error.h"
#include "utils.h"
#include "terminal.h"
#include "serial_io.h"
#include "gps.h"
#include "gpsd.h"

gpsd::gpsd(std::string host_in, int port_in, terminal *t_in)
{
	fake = abort = valid = false;

	host = host_in;
	port = port_in;

	t = t_in;

	la = lo = al = 0.0;
	fake_la = fake_lo = 45.0;
	fake_al = 50.0;

	sig_db = -1;
	n_sats = -1;

	speed = -1.0;

	prev_lo = prev_la = prev_al = prev_ts = -1.0;

#if RPI == 1
	init_rpi_light();
#endif

	pthread_mutex_init(&lck, NULL);

	pthread_create(&tid, NULL, start_gps_thread_wrapper, this);
}

gpsd::~gpsd()
{
}

void gpsd::run()
{
	struct gps_data_t gd;

	std::string port_str = format("%d", port);

	if (gps_open(host.c_str(), port_str.c_str(), &gd) == -1)
		error_exit("gps_open failed");

	(void)gps_stream(&gd, WATCH_ENABLE | WATCH_JSON, NULL);

	for(;;)
	{
                pthread_mutex_lock(&lck);
                bool dummy_abort = abort;
                pthread_mutex_unlock(&lck);
                if (dummy_abort)
                        break;

		if (!gps_waiting(&gd, 250000))
			continue;

		if (gps_read(&gd) == -1)
			error_exit("GPSd read error");

		ts = get_ts();

		pthread_mutex_lock(&lck);

		n_sats = gd.satellites_visible;

		if (gd.set & SPEED_SET)
			speed = gd.fix.speed;
		else
			speed = fabs(haversine_km(la, lo, prev_la, prev_lo) * 3600.0) / (ts - prev_ts);

		sig_db = 0; // not known

		valid = false;
		if (gd.set & LATLON_SET)
		{
			lo = gd.fix.longitude;
			la = gd.fix.latitude;

			if (gd.status != STATUS_NO_FIX)
				valid = true;
		}
		else
		{
			prev_la = prev_lo = lo = la = 0.0;
		}

		if (gd.set & ALTITUDE_SET)
			al = gd.fix.altitude;
		else
			al = 0.0;

		nmea_coords = format("%9f,%9f,%9f", lo, la, al);

		pthread_mutex_unlock(&lck);

		prev_la = la;
		prev_lo = lo;
		prev_al = al;
		prev_ts = ts;

#if RPI == 1
		pulse_rpi_light(t);
#endif
	}
}

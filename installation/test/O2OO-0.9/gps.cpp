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

#include "error.h"
#include "utils.h"
#include "terminal.h"
#include "serial_io.h"
#include "gps.h"

void *start_gps_thread_wrapper(void *p)
{
	gps *g = reinterpret_cast<gps *>(p);

	g -> run();

	return NULL;
}

gps::gps(serial_io *sio_in, terminal *t_in) : sio(sio_in), t(t_in)
{
	fake = abort = valid = false;

	la = lo = al = 0.0;
	fake_la = fake_lo = 45.0;
	fake_al = 50.0;

	sig_db = -1;
	n_sats = -1;

	speed = -1.0;

	prev_lo = prev_la = prev_al = prev_ts = -1.0;

	pthread_mutex_init(&lck, NULL);

	if (!sio_in)
		fake = true;

	pthread_create(&tid, NULL, start_gps_thread_wrapper, this);
}

gps::~gps()
{
	t -> emit(LL_INFO, "Waiting for GPS thread to exit...");

	pthread_mutex_lock(&lck);
	abort = true;
	pthread_mutex_unlock(&lck);

	pthread_join(tid, NULL);

	pthread_mutex_destroy(&lck);

	t -> emit(LL_INFO, "GPS thread terminated");
}

void gps::run()
{
	char buffer[128] = { 0 }; // a sentence can be at most 82 characters (including cr/lf)
	bool garbage = false;

	double to_rad = PI / 180.0;
	bool have_gps_speed = false;
	for(;;)
	{
		pthread_mutex_lock(&lck);
		bool dummy_abort = abort;
		pthread_mutex_unlock(&lck);
		if (dummy_abort)
			break;

		if (fake)
		{
			usleep(100000);

			// FIXME checksum!
			std::string nmea_val_lo, nmea_val_la;
			char nmea_char_lo, nmea_char_la;
			degrees_to_nmea(fake_lo, &nmea_val_lo, true, &nmea_char_lo);
			degrees_to_nmea(fake_la, &nmea_val_la, false, &nmea_char_la);
			int len = snprintf(buffer, sizeof buffer, "$GPGGA,170834,%s,%c,%s,%c,1,05,1.5,%.1f,M,-34.0,M,,,*",
				nmea_val_lo.c_str(), nmea_char_lo, nmea_val_la.c_str(), nmea_char_la, fake_al);

			double co = fabs(cos(get_ts() * to_rad)) / 64.0;
			double si = sin(get_ts() * to_rad) / 64.0;
			fake_lo += co;
			fake_la += si;
			fake_al += co * si * (drand48() * 2.0 - 1.0);

			if (fake_lo <= -90.0)
				fake_lo = -89.9;
			if (fake_la <= -90.0)
				fake_la = -89.9;
			if (fake_lo >= 90.0)
				fake_lo = 89.9;
			if (fake_la >= 90.0)
				fake_la = 89.9;

			if (fake_al > 100.0)
				fake_al = 100.0;
			else if (fake_al < 0.0001)
				fake_al = 0.0001;

			char chk = 0;
			for(int index=1; index<len; index++)
				chk ^= index;

			snprintf(&buffer[len], sizeof buffer - len, "%02x", chk);
		}
		else
		{
			int index = 0;

			for(;;)
			{
				pthread_mutex_lock(&lck);
				dummy_abort = abort;
				pthread_mutex_unlock(&lck);
				if (dummy_abort)
					break;

				int rc = sio -> read(&buffer[index], 1);
				if (rc == 0)
					error_exit("Connection to GPS lost?");

				if (rc == -1)
				{
					if (errno == EINTR || errno == EAGAIN)
						continue;

					error_exit("Problem receiving data from GPS");
				}

				if (buffer[index] == 13 || buffer[index] == 10)
				{
					buffer[index] = 0x00;
					break;
				}

				index++;

				if (index == sizeof buffer)
					index = 0;
			}
		}

		if (buffer[0] == 0x00)
			continue;

		std::vector<std::string> sentence_parts = split_string(buffer, ",");

		if (sentence_parts.size() <= 2)
		{
			if (!garbage)
			{
				garbage = true;

				t -> emit(LL_ERROR, "Receiving garbage from GPS?! Is it configured at 4800,n,8,1?");
			}

			continue;
		}

		pthread_mutex_lock(&lck);
		ts = get_ts();

		// $GPGGA,170834,4124.8963,N,08151.6838,W,1,05,1.5,280.2,M,-34.0,M,,,*59
		bool used = false;
		if (sentence_parts.at(0) == "$GPGGA" && sentence_parts.size() >= 9 + 1)
		{
			used = valid = sentence_parts.at(6) != "0";

			t -> emit(LL_DEBUG_SOME, buffer + std::string(valid ? " FIX!" : ""));

			n_sats = atoi(sentence_parts.at(7).c_str());

			if (prev_ts > 0.0 && !have_gps_speed)
				speed = fabs(haversine_km(la, lo, prev_la, prev_lo) * 3600.0) / (ts - prev_ts);

			la = nmea_to_degrees(sentence_parts.at(2), sentence_parts.at(3));
			lo = nmea_to_degrees(sentence_parts.at(4), sentence_parts.at(5));
			al = atof(sentence_parts.at(9).c_str());

			nmea_coords = sentence_parts.at(2) + sentence_parts.at(3) + "," + sentence_parts.at(4) + sentence_parts.at(5) + "," + sentence_parts.at(9);
		}
		else if (sentence_parts.at(0) == "$GPRMC" && sentence_parts.size() >= 7 + 1)
		{
			used = valid = sentence_parts.at(2) == "A";

			t -> emit(LL_DEBUG_SOME, buffer + std::string(valid ? " FIX!" : ""));

			la = nmea_to_degrees(sentence_parts.at(3), sentence_parts.at(4));
			lo = nmea_to_degrees(sentence_parts.at(5), sentence_parts.at(6));

			have_gps_speed = true;
			speed = atof(sentence_parts.at(7).c_str()) / 1.852;

			nmea_coords = sentence_parts.at(3) + sentence_parts.at(4) + "," + sentence_parts.at(5) + sentence_parts.at(6);
		}
		else if (sentence_parts.at(0) == "$GPGLL" && sentence_parts.size() >= 4 + 1)
		{
			t -> emit(LL_DEBUG_SOME, buffer);

			// 5 fields implies valid
			used = valid = true;

			if (prev_ts > 0.0 && !have_gps_speed)
				speed = fabs(haversine_km(la, lo, prev_la, prev_lo) * 3600.0) / (ts - prev_ts);

			la = nmea_to_degrees(sentence_parts.at(1), sentence_parts.at(2));
			lo = nmea_to_degrees(sentence_parts.at(3), sentence_parts.at(4));

			nmea_coords = sentence_parts.at(1) + sentence_parts.at(2) + "," + sentence_parts.at(3) + sentence_parts.at(4);
		}
		else if (sentence_parts.at(0) == "$GPGSA" && sentence_parts.size() >= 15)
		{
			if (sentence_parts.at(2) != "1") // FIX
				used = valid = true;

			t -> emit(LL_DEBUG_SOME, buffer + std::string(valid ? " FIX!" : ""));

			n_sats = 0;
			for(int index=3; index<15; index++)
			{
				if (sentence_parts.at(index) != "")
					n_sats++;
			}
		}
		else if (sentence_parts.at(0) == "$GPVTG" && sentence_parts.size() >= 7 + 1)
		{
			t -> emit(LL_DEBUG_SOME, buffer);

			used = true;

			have_gps_speed = true;
			speed = atof(sentence_parts.at(7).c_str());
		}
		else if (sentence_parts.at(0) == "$GPGSV" && sentence_parts.size() >= 7 + 1)
		{
			t -> emit(LL_DEBUG_SOME, buffer);

			used = true;

			double value = atof(sentence_parts.at(7).c_str());
			if (value != 0.0) // 0.0 = not measured
				sig_db = value;
		}
		else
		{
			t -> emit(LL_ERROR, format("ignoring: %s", buffer));
		}

		if (used && valid)
		{
			prev_la = la;
			prev_lo = lo;
			prev_al = al;
			prev_ts = ts;
		}

		pthread_mutex_unlock(&lck);
	}
}

bool gps::get_coordinates(double *ts_out, double *lo_out, double *la_out, double *al_out)
{
	bool rc = false;

	pthread_mutex_lock(&lck);
	rc = valid;
	*ts_out = ts;
	*lo_out = lo;
	*la_out = la;
	*al_out = al;
	pthread_mutex_unlock(&lck);

	return rc;
}

double gps::nmea_to_degrees(std::string val_in, std::string ch)
{
	std::vector<std::string> parts = split_string(val_in, ".");

	if (parts.empty())
		return 0;

	std::string secs = parts.size() == 2 ? parts.at(1) : "0";

	std::string p1 = parts.at(0);
	int len = p1.size();
	if (len < 2)
		return 0;

	std::string degr, min;
	if (len > 2)
	{
		degr = p1.substr(0, len - 2);
		min = p1.substr(len - 2);
	}
	else
	{
		degr = p1.substr(0, len - 2);
		min = "0";
	}

	double val = atof(degr.c_str()) + atof((min + "." + secs).c_str()) / 60.0;

	if (ch == "S" || ch == "W")
		return -val;

	return val; // N/E
}

void gps::degrees_to_nmea(double deg, std::string *nmea_val, bool lo, char *nmea_char)
{
	double deg_int = floor(deg); 
	double minutes = deg - deg_int;   

	double val = minutes * 60 + deg_int * 10.0 * 10.0;
	nmea_val -> assign(format("%.4f", fabs(val)));

	if (val < 0)
		*nmea_char = lo ? 'S' : 'W';
	else
		*nmea_char = lo ? 'N' : 'E';
}

bool gps::get_nmea_coordinates(std::string *out)
{
	pthread_mutex_lock(&lck);
	bool rc = valid;
	out -> assign(nmea_coords);
	pthread_mutex_unlock(&lck);

	return rc;
}

bool gps::has_fix()
{
	pthread_mutex_lock(&lck);
	bool rc = valid;
	pthread_mutex_unlock(&lck);

	return rc;
}

int gps::get_n_satellites_in_view()
{
	pthread_mutex_lock(&lck);
	int n = n_sats;
	pthread_mutex_unlock(&lck);

	return n;
}

double gps::get_ground_speed()
{
	pthread_mutex_lock(&lck);
	double ret = speed;
	pthread_mutex_unlock(&lck);

	return ret;
}

double gps::get_signal_strength()
{
	pthread_mutex_lock(&lck);
	double ret = sig_db;
	pthread_mutex_unlock(&lck);

	return ret;
}

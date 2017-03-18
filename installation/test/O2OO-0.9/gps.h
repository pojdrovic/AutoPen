void *start_gps_thread_wrapper(void *p);

class gps
{
private:
	serial_io *sio;

protected:
	terminal *t;
	pthread_mutex_t lck;
	double ts, lo, la, al;
	double prev_ts, prev_lo, prev_la, prev_al;
	bool valid;
	std::string nmea_coords;
	int n_sats;
	double speed, sig_db;

	pthread_t tid;
	volatile bool abort;

	bool fake;
	double fake_lo, fake_la, fake_al;

	gps() { }

public:
	gps(serial_io *sio, terminal *t_in);
	virtual ~gps();

	static double nmea_to_degrees(std::string val, std::string ch);
	static void degrees_to_nmea(double deg, std::string *nmea_val, bool lo, char *nmea_char);

	virtual void run(void);

	bool get_coordinates(double *ts_out, double *lo_out, double *la_out, double *al_out);
	bool get_nmea_coordinates(std::string *out);
	bool has_fix();
	int get_n_satellites_in_view();
	double get_ground_speed(); // km/h
	double get_signal_strength(); // db
};

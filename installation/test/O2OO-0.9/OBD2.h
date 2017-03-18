// SVN: $Revision: 464 $
class OBD2
{
private:
	IO_Device *io;
	terminal *t;
	bool is_supported[256];
	int n_ok, n_fail;
	double avg_ts;

	std::map<std::string, sensor *> symbol_to_object_available;
	std::map<int, sensor *> mode_pid_to_object_available;

	std::map<std::string, sensor *> symbol_to_object_detected;

	void init_pid_map();
	void determine_supported_mode_0x01_pids();

	std::map<std::string, sensor *> * get_available_sensors() { return &symbol_to_object_available; }

	void add_sensor(sensor *s);
	void add_io_sensor(io_sensor *s);

	bool get_string(int mode, int pid, std::string *out);

public:
	OBD2(IO_Device *io_in, terminal *t);

	std::map<std::string, sensor *> * get_detected_sensors() { return &symbol_to_object_detected; }

	bool get_is_supported(int mode, int pid);

	bool get_pid_16b(int pid, int *ret);
	bool get_pid_16b_signed(int pid, int *ret);

	bool get_pid_8b(int pid, int *ret, int offset = 0);
	bool get_pid_8b_a(int pid, double *ret);
	bool get_pid_8b_b(int pid, double *ret);
	bool get_pid_8b_c(int pid, double *ret);
	bool get_pid_8b_mul10_d(int pid, double *ret);

	byte_array * get_pid(int pid);
	bool get_pid_percent(int pid, double *value);
	bool get_pid_percent128(int pid, double *value);
	bool get_pid_mul3(int pid, double *value);
	bool get_pid_16b_div4(int pid, double *value);
	bool get_pid_16b_div100(int pid, double *value);
	bool get_pid_div2_sub64(int pid, double *value);
	bool get_pid_16b_div2(int pid, double *value);
	bool get_pid_16b_div32768(int pid, double *value);
	bool get_pid_sub40(int pid, double *value);
	bool get_pid_16b_percent_25700(int pid, double *value);
	bool get_pid_sub125(int pid, double *value);
	bool get_pid_16b_div10_sub40(int pid, double *value);
	bool get_pid_double(int pid, double *value);
	bool get_pid_16b_double(int pid, double *value);
	bool get_pid_3B_div200(int pid, double *value);
	bool get_pid_16b_div_1000(int pid, double *value);
	bool get_pid_16b_mul0_079(int pid, double *value);
	bool get_pid_16b_mul10(int pid, double *value);
	bool get_pid_16b_sub26880_div128(int pid, double *value);
	bool get_pid_16b_mul0_05(int pid, double *value);
	bool get_pid_ab_div32768(int pid, double *value);
	bool get_pid_cd_div8192(int pid, double *value);
	bool get_pid_16b(int pid, double *ret);

	bool get_is_supported(int index) const { return is_supported[index]; }

	bool ignore_sensor(std::string);

	void list_supported_pids();

	void get_io_stats(int *n_ok, int *n_fail, double *avg_resp_time) const;

	bool get_io_sensor(std::string symbol, double *value);

	bool get_VIN(std::string *out);
	bool get_calibration_ID(std::string *out);
};

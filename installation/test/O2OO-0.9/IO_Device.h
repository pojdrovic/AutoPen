// SVN: $Revision: 463 $
class IO_Device
{
protected:
	terminal *t;
	bool full_speed;
	double last_msg;

	int n_sr_ok, n_sr_fail;

public:
	IO_Device(terminal *t, bool full_speed);
	virtual ~IO_Device();

	void slowdown();

	virtual void flush() = 0 ;

	virtual std::string get_source_type() = 0;

	virtual bool has_sensor(std::string s) = 0;
	virtual bool get_sensor(std::string s, double *value) = 0;

	virtual void send_recv(byte_array *command, std::vector<byte_array *> *results, int result_count_limit = -1) = 0;

	void get_io_stats(int *n_ok, int *n_fail) const;
};

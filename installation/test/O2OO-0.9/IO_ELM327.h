// SVN: $Revision: 463 $
class IO_ELM327 : public IO_Device
{
private:
	serial_io *sio;
	std::string dev_info;

	void reset_device();
	std::string recv();
	void wait_for_prompt();

public:
	IO_ELM327(serial_io *sio, terminal *t, bool full_speed);
	~IO_ELM327();

	void flush() { sio -> flush(); }

	bool has_sensor(std::string s);
	bool get_sensor(std::string s, double *value);

	std::string get_source_type();

	void send_recv(byte_array *command, std::vector<byte_array *> *results, int result_count_limit = -1);
};

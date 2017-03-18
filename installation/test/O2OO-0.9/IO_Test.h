// SVN: $Revision: 463 $
class IO_Test : public IO_Device
{
public:
	IO_Test(terminal *t, bool full_speed);
	virtual ~IO_Test();

	void flush() { }

	bool has_sensor(std::string s);
	bool get_sensor(std::string s, double *value);

	std::string get_source_type();

	void send_recv(byte_array *command, std::vector<byte_array *> *results, int result_count_limit = -1);
};

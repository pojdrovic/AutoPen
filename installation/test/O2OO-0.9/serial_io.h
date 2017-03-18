class serial_io
{
protected:
	std::string dev;
	int fd;

	void open_device();
	serial_io();

public:
	serial_io(std::string dev_in, int bps);
	virtual ~serial_io();

	virtual void flush();
	virtual int read(char *where, size_t n);
	virtual int write(const char *what, size_t n);
};

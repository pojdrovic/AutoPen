class serial_io_tcpip : public serial_io
{
protected:
	std::string host;
	int port;

	void reconnect();

public:
	serial_io_tcpip(std::string host, int port);
	~serial_io_tcpip();

	void flush();
	int read(char *where, size_t n);
	int write(const char *what, size_t n);
};

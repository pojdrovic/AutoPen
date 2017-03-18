class gpsd : public gps
{
private:
	std::string host;
	int port;

public:
	gpsd(std::string host_in, int port_in, terminal *t_in);
	~gpsd();

	void run(void);
};

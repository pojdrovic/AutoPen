typedef enum {
	ppid_16b,
	ppid_16b_div_1000,
	ppid_16b_double,
	ppid_16b_div100,
	ppid_16b_div10_sub40,
	ppid_16b_div4,
	ppid_16b_percent_25700,
	ppid_16b_div2,
	ppid_16b_div32768,
	ppid_16b_mul0_05,
	ppid_16b_mul0_079,
	ppid_16b_mul10,
	ppid_16b_sub26880_div128,
	ppid_3B_div200,
	ppid_8b_a,
	ppid_8b_b,
	ppid_8b_c,
	ppid_8b_mul10_d,
	ppid_ab_div32768,
	ppid_cd_div8192,
	ppid_div2_sub64,
	ppid_mul3,
	ppid_percent,
	ppid_percent128,
	ppid_pid_double,
	ppid_string,
	ppid_sub125,
	ppid_sub40,
	io_get
} processing_t;

class OBD2;

class sensor
{
private:
	int error_count;

public:
	sensor();
	virtual ~sensor();

	// meta data
	virtual int get_mode() = 0;
	virtual int get_pid() = 0;
	virtual std::string get_symbol() = 0;
	virtual std::string get_unit() = 0;
	virtual std::string get_description() = 0;
        virtual std::string get_screen_formatting() = 0;
        virtual color_t get_color() = 0;
	virtual void get_range(double *mi, double *ma) = 0;
	virtual processing_t get_processor() = 0;
	virtual bool is_meta() = 0; // e.g. request once and put in 
	virtual bool get_value(terminal *t, OBD2 *io, std::string *value);

	// data
	bool get_value(terminal *t, OBD2 *io, double *value);
};

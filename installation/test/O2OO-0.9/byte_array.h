// SVN: $Revision: 450 $
class byte_array
{
private:
	unsigned char *bytes;
	int n;

public:
	byte_array();
	byte_array(unsigned char *bytes, int n);
	byte_array(std::string hex);
	~byte_array();

	void clear() { n = 0; }

	int size() const { return n; }
	void grow(int new_size);

	void set(char *bytes, int n);
	void set(unsigned char *bytes, int n);
	void set(int byte, char value);
	void set(int byte, unsigned char value);
	void set(int byte, int value);

	void set_bit(int bit);
	void set_bit_le(int bit);
	void reset_bit(int bit_in);
	void reset_bit_le(int bit_in);
	bool get_bit(int bit);
	bool get_bit_le(int bit);

	char get_char(int byte);
	unsigned char get_uchar(int byte);
};

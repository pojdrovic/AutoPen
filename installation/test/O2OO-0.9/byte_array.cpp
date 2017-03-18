// SVN: $Revision: 450 $
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <vector>

#include "error.h"
#include "byte_array.h"
#include "utils.h"

byte_array::byte_array() : bytes(NULL), n(0)
{
}

byte_array::byte_array(std::string hex) : bytes(NULL), n(0)
{
	std::vector<std::string> parts = split_string(hex, " ");

	n = parts.size();
	bytes = (unsigned char *)malloc(n);

	for(int index=0; index<n; index++)
		bytes[index] = strtol(parts.at(index).c_str(), NULL, 16);
}

byte_array::byte_array(unsigned char *bytes_in, int n_in)
{
	n = n_in;
	bytes = (unsigned char *)malloc(n);

	memcpy(bytes, bytes_in, n);
}

byte_array::~byte_array()
{
	free(bytes);
}

void byte_array::grow(int new_size)
{
	new_size++; // this function gets an offset

	if (new_size > n)
	{
		bytes = (unsigned char *)realloc(bytes, new_size);
		if (!bytes)
			error_exit("Problem allocating %d bytes", new_size);

		memset(&bytes[n], 0x00, new_size - n);

		n = new_size;
	}
}

void byte_array::set(char *bytes_in, int n_in)
{
	set((unsigned char *)bytes_in, n_in);
}

void byte_array::set(unsigned char *bytes_in, int n_in)
{
	free(bytes);

	bytes = (unsigned char *)malloc(n_in);
	memcpy(bytes, bytes_in, n_in);
	n = n_in;
}

void byte_array::set(int byte, char value)
{
	set(byte, (unsigned char)value);
}

void byte_array::set(int byte, unsigned char value)
{
	grow(byte);

	bytes[byte] = value;
}

void byte_array::set(int byte, int value)
{
	set(byte, (unsigned char)value);
}

void byte_array::set_bit(int bit_in)
{
	int byte = bit_in / 8;
	int bit = bit_in & 7;

	grow(byte);

	bytes[byte] |= 1 << bit;
}

void byte_array::set_bit_le(int bit_in)
{
	int byte = bit_in / 8;
	int bit = 7 - (bit_in & 7);

	grow(byte);

	bytes[byte] |= 1 << bit;
}

void byte_array::reset_bit(int bit_in)
{
	int byte = bit_in / 8;
	int bit = bit_in & 7;

	grow(byte);

	bytes[byte] &= ~(1 << bit);
}

void byte_array::reset_bit_le(int bit_in)
{
	int byte = bit_in / 8;
	int bit = 7 - (bit_in & 7);

	grow(byte);

	bytes[byte] &= ~(1 << bit);
}

bool byte_array::get_bit(int bit_in)
{
	int byte = bit_in / 8;
	int bit = bit_in & 7;

	grow(byte);

	return bytes[byte] & (1 << bit) ? true : false;
}

bool byte_array::get_bit_le(int bit_in)
{
	int byte = bit_in / 8;
	int bit = bit_in & 7;

	grow(byte);

	return bytes[byte] & (1 << (7 - bit)) ? true : false;
}

char byte_array::get_char(int byte)
{
	grow(byte);

	return (char)bytes[byte];
}

unsigned char byte_array::get_uchar(int byte)
{
	grow(byte);

	return bytes[byte];
}

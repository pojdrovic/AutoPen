// SVN: $Revision: 442 $
#include <math.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "byte_array.h"
#include "terminal.h"
#include "utils.h"

void emulate_obd2(terminal *t, byte_array *command, std::vector<byte_array *> *results)
{
	results -> clear();

	if (command -> get_uchar(0) != 0x01) // only handle current sensor data
		return;

	int pid = command -> get_uchar(1);

	byte_array *out = new byte_array();
	out -> set(0, 0x41);
	out -> set(1, pid);

	byte_array map;

	switch(pid)
	{
		case 0x00:	// pids supported
			t -> emit(LL_DEBUG_SOME, "PID 0x00");
			map.clear();
			map.set_bit_le(0x03 - 1);
			map.set_bit_le(0x04 - 1);
			map.set_bit_le(0x05 - 1);
			map.set_bit_le(0x07 - 1);
			map.set_bit_le(0x09 - 1);
			map.set_bit_le(0x0a - 1);
			map.set_bit_le(0x0b - 1);
			map.set_bit_le(0x0d - 1);
			map.set_bit_le(0x0f - 1);
			map.set_bit_le(0x11 - 1);
			map.set_bit_le(0x1c - 1);
			map.set_bit_le(0x20 - 1); // 0x20-0x3f
			out -> set(2, map.get_uchar(0));
			out -> set(3, map.get_uchar(1));
			out -> set(4, map.get_uchar(2));
			out -> set(5, map.get_uchar(3));
			break;

		case 0x04:
		case 0x05:
		case 0x07:
		case 0x09:
		case 0x0a:
		case 0x0b:
		case 0x0d:
		case 0x0f:
		case 0x11:
		case 0x1c:
		case 0x30:
		case 0x33:
		case 0x45:
		case 0x46:
		case 0x52:
		case 0x5c:
			t -> emit(LL_DEBUG_SOME, format("PID 0x%02x", pid));

			out -> set(2, rand() & 255); // some arbitrary value
			break;

		case 0x03:
		case 0x21:
			t -> emit(LL_DEBUG_SOME, format("PID 0x%02x", pid));

			out -> set(2, rand() & 255); // some arbitrary value
			out -> set(3, rand() & 255); // some arbitrary value
			break;

		case 0x20:
			t -> emit(LL_DEBUG_SOME, "PID 0x20");
			map.clear();
			map.set_bit_le(0x21 - 0x20 - 1);
			map.set_bit_le(0x30 - 0x20 - 1);
			map.set_bit_le(0x33 - 0x20 - 1);
			map.set_bit_le(0x40 - 0x20 - 1); // 0x40-0x5f
			out -> set(2, map.get_uchar(0));
			out -> set(3, map.get_uchar(1));
			out -> set(4, map.get_uchar(2));
			out -> set(5, map.get_uchar(3));
			break;

		case 0x40:
			t -> emit(LL_DEBUG_SOME, "PID 0x40");
			map.clear();
			map.set_bit_le(0x45 - 0x40 - 1);
			map.set_bit_le(0x46 - 0x40 - 1);
			map.set_bit_le(0x52 - 0x40 - 1);
			map.set_bit_le(0x5c - 0x40 - 1);
			out -> set(2, map.get_uchar(0));
			out -> set(3, map.get_uchar(1));
			out -> set(4, map.get_uchar(2));
			out -> set(5, map.get_uchar(3));
			break;

		default:
			out -> clear();
			t -> emit(LL_ERROR, "*** UNSUPPORTED PID ***");
			return; // unsupported pid
	}

	results -> push_back(out);
}

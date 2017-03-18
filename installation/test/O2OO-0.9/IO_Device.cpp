// SVN: $Revision: 442 $
#include <string>
#include <unistd.h>
#include <vector>

#include "byte_array.h"
#include "terminal.h"
#include "IO_Device.h"
#include "utils.h"

IO_Device::IO_Device(terminal *t_in, bool full_speed_in) : t(t_in), full_speed(full_speed_in)
{
	n_sr_ok = n_sr_fail = 0;
	last_msg = 0;
}

IO_Device::~IO_Device()
{
}

void IO_Device::get_io_stats(int *n_ok, int *n_fail) const
{
	*n_ok = n_sr_ok;
	*n_fail = n_sr_fail;
}

void IO_Device::slowdown()
{
	if (!full_speed)
	{
		double now = get_ts();
		double sleep = now - last_msg;

		if (sleep < 0.1) // must have at least 0.1s between each request (requirement for vehicles < APR2002)
		{
			t -> emit(LL_DEBUG_ALL, format("Sleeping %f seconds", 0.1 - sleep));

			usleep(useconds_t((0.1 - sleep) * 1000000.0));
		}
	}

	last_msg = get_ts();
}

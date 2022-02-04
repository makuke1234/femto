#include "femtoErr.h"
#include "common.h"

static const char * femto_errCodes[femtoErr_num_of_elems] = {
	[femtoErr_ok]      = "Everything OK",
	[femtoErr_unknown] = "Uknown error occurred!",
	[femtoErr_file]    = "Error reading file!",
	[femtoErr_window]  = "Error initialising window!",
	[femtoErr_memory]  = "Error allocating memory!"
};

void femto_printErr(femtoErr_t errCode)
{
	if (errCode >= femtoErr_num_of_elems)
	{
		errCode = femtoErr_unknown;
	}
	fprintf(stderr, "%s\n", femto_errCodes[errCode]);
}

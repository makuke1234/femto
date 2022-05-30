#include "femtoErr.h"
#include "common.h"

static const char * s_femto_errCodes[ferrNUM_OF_ELEMS] = {
	[ferrOK]      = "Everything OK",
	[ferrUNKNOWN] = "Uknown error occurred!",
	[ferrFILE]    = "Error reading file!",
	[ferrWINDOW]  = "Error initialising window!",
	[ferrMEMORY]  = "Error allocating memory!",
	[ferrTHREAD]  = "Error creating thread!"
};

void femto_printErr(femtoErr_e errCode)
{
	if (errCode >= ferrNUM_OF_ELEMS)
	{
		errCode = ferrUNKNOWN;
	}
	fprintf(stderr, "%s\n", s_femto_errCodes[errCode]);
}

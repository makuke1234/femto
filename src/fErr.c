#include "fErr.h"
#include "fCommon.h"

static const char * s_fErrCodes[ferrNUM_OF_ELEMS] = {
	[ferrOK]      = "Everything OK",
	[ferrUNKNOWN] = "Uknown error occurred!",
	[ferrFILE]    = "Error reading file!",
	[ferrWINDOW]  = "Error initialising window!",
	[ferrMEMORY]  = "Error allocating memory!",
	[ferrTHREAD]  = "Error creating thread!"
};

void fErr_print(fErr_e errCode)
{
	errCode = (errCode >= ferrNUM_OF_ELEMS) ? ferrUNKNOWN : errCode;
	fprintf(stderr, "%s\n", s_fErrCodes[errCode]);
}

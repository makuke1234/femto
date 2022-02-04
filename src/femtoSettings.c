#include "femtoSettings.h"
#include "femtoArg.h"
#include "femto.h"


void femtoSettings_reset(femtoSettings_t * restrict self)
{
	assert(self != NULL);
	*self = (femtoSettings_t){
		.helpRequested = false,
		.fileName      = NULL,



		.lastErr       = { 0 }
	};
}

bool femtoSettings_getLastError(femtoSettings_t * restrict self, wchar_t * restrict errArr, uint32_t errMax)
{
	assert(self != NULL);
	assert(errArr != NULL);
	if (self->lastErr[0] == L'\0')
	{
		errArr[0] = L'\0';
		return false;
	}
	uint32_t copyLen = u32Min((uint32_t)wcslen(self->lastErr) + 1, errMax);
	memcpy(errArr, self->lastErr, sizeof(wchar_t) * copyLen);
	self->lastErr[copyLen - 1] = L'\0';
	return true;
}

femtoErr_t femtoSettings_populate(femtoSettings_t * restrict self, int argc, const wchar_t ** restrict argv)
{
	assert(self != NULL);
	assert(argv != NULL);

	bool * argumentsUsed = calloc(argc - 1, sizeof(bool));
	if (argumentsUsed == NULL)
	{
		return femtoErr_memory;
	}

	uint32_t mi;
	// Search for help argument first
	femtoArg_fetchArgv(argc, argv, L"help", &mi, 0);
	if (mi != 0)
	{
		self->helpRequested = true;
		free(argumentsUsed);
		return true;
	}
	femtoArg_t fname;
	femtoArg_fetchArgv(argc, argv, L"file", &mi, 1, &fname);
	if (mi != 0)
	{
		self->fileName = wcsdup_s(fname.begin, fname.end - fname.begin);
		if (self->fileName == NULL)
		{
			free(argumentsUsed);
			return femtoErr_memory;
		}
		argumentsUsed[mi - 1] = true;
	}

	// Other settings



	// If filename wasn't specified and the last argument isn't used, take the last argument
	// If the last argument is already in use, leave fileName as NULL, opens an empty new file
	if ((self->fileName == NULL) && !argumentsUsed[argc - 2])
	{
		self->fileName = wcsdup_s(argv[argc - 1], SIZE_MAX);
		if (self->fileName == NULL)
		{
			free(argumentsUsed);
			return femtoErr_memory;
		}
		// Just in case, redundant otherwise
		argumentsUsed[argc - 2] = true;
	}

	// Everything is OK
	free(argumentsUsed);
	return femtoErr_ok;
}

void femtoSettings_destroy(femtoSettings_t * restrict self)
{
	if (self->fileName != NULL)
	{
		free(self->fileName);
		self->fileName = NULL;
	}
	// Clear last error, just in case
	self->lastErr[0] = L'\0';
}

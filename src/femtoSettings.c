#include "femtoSettings.h"
#include "femtoArg.h"
#include "femto.h"


void femtoSettings_reset(femtoSettings_t * restrict self)
{
	assert(self != NULL);
	*self = (femtoSettings_t){
		.helpRequested    = false,
		.fileName         = NULL,
		.settingsFileName = NULL,

		.tabsToSpaces = false,
		.tabWidth     = 4,
		.tabSpaceStr  = NULL,


		.lastErr = { 0 }
	};
	if (!femtoSettings_makeTabSpaceStr(self))
	{
		fprintf(stderr, "Cannot recover from this memory allocation error!\n");
		exit(1);
	}
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

bool femtoSettings_makeTabSpaceStr(femtoSettings_t * restrict self)
{
	wchar_t * mem = realloc(self->tabSpaceStr, sizeof(wchar_t) * (self->tabWidth + 1));
	if (mem == NULL)
	{
		return false;
	}
	self->tabSpaceStr[self->tabWidth] = L'\0';
	for (uint8_t i = 0; i < self->tabWidth; ++i)
	{
		self->tabSpaceStr[i] = L' ';
	}
	self->tabSpaceStr = mem;
	return true;
}

femtoErr_t femtoSettings_populate(femtoSettings_t * restrict self, int argc, const wchar_t ** restrict argv)
{
	assert(self != NULL);
	assert(argc > 0);
	assert(argv != NULL);

	bool * argumentsUsed = calloc((size_t)(argc - 1), sizeof(bool));
	if (argumentsUsed == NULL)
	{
		return femtoErr_memory;
	}

	int mi;
	// Search for help argument first
	femtoArg_fetchArgv(argc, argv, L"help", &mi, 0);
	if (mi != 0)
	{
		self->helpRequested = true;
		free(argumentsUsed);
		return femtoErr_ok;
	}
	femtoArg_t fname;
	femtoArg_fetchArgv(argc, argv, L"file", &mi, 1, &fname);
	if (mi != 0)
	{
		self->fileName = wcsdup_s(fname.begin, (size_t)(fname.end - fname.begin));
		if (self->fileName == NULL)
		{
			free(argumentsUsed);
			return femtoErr_memory;
		}
		argumentsUsed[mi - 1] = true;
	}

	/* **************** Other settings ******************* */

	femtoArg_t sfname;
	femtoArg_fetchArgv(argc, argv, L"settings", &mi, 1, &sfname);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"setting", &mi, 1, &sfname);
	}
	if (mi != 0)
	{
		wchar_t * mem = wcsdup_s(sfname.begin, (size_t)(sfname.end - sfname.begin));
		if (mem == NULL)
		{
			free(argumentsUsed);
			return femtoErr_memory;
		}
		argumentsUsed[mi - 1] = true;

		if (femto_testFile(mem))
		{
			self->settingsFileName = mem;
		}
		else
		{
			free(mem);
		}
	}

	femtoArg_t tabs;
	femtoArg_fetchArgv(argc, argv, L"tabsSpaces", &mi, 1, &tabs);
	if (mi != 0)
	{
		if (((tabs.end - tabs.begin) >= 4) && (wcsncmp(tabs.begin, L"true", 4) == 0))
		{
			self->tabsToSpaces = true;
		}
		else if (((tabs.end - tabs.begin) >= 5) && (wcsncmp(tabs.begin, L"false", 5) == 0))
		{
			self->tabsToSpaces = false;
		}
		else
		{
			// String to int conversion
			self->tabsToSpaces = wcstol(tabs.begin, NULL, 10) != 0;
		}
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"tabWidth", &mi, 1, &tabs);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"tabw", &mi, 1, &tabs);
	}
	if (mi != 0)
	{
		self->tabWidth = (uint8_t)u32Clamp((uint32_t)wcstol(tabs.begin, NULL, 10), 1, 32);
		if (!femtoSettings_makeTabSpaceStr(self))
		{
			free(argumentsUsed);
			return femtoErr_memory;
		}

		argumentsUsed[mi - 1] = true;
	}

	/* *************************************************** */

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
	if (self->settingsFileName == NULL)
	{
		if (femto_testFile(FEMTO_SETTINGS_FILE1))
		{
			self->settingsFileName = wcsdup(FEMTO_SETTINGS_FILE1);
		}
		else if (femto_testFile(FEMTO_SETTINGS_FILE2))
		{
			self->settingsFileName = wcsdup(FEMTO_SETTINGS_FILE2);
		}
		else if (femto_testFile(FEMTO_SETTINGS_FILE3))
		{
			self->settingsFileName = wcsdup(FEMTO_SETTINGS_FILE3);
		}
		else if (femto_testFile(FEMTO_SETTINGS_FILE4))
		{
			self->settingsFileName = wcsdup(FEMTO_SETTINGS_FILE4);
		}
	}

	// try to load settings from file
	femtoSettings_loadFromFile(self);

	// Everything is OK
	free(argumentsUsed);
	return femtoErr_ok;
}

femtoErr_t femtoSettings_loadFromFile(femtoSettings_t * restrict self)
{
	// Generate default values
	femtoSettings_t def;
	femtoSettings_reset(&def);

	// Try to load file, for every value, change it only if it's default value


	return femtoErr_ok;
}


void femtoSettings_destroy(femtoSettings_t * restrict self)
{
	if (self->fileName != NULL)
	{
		free(self->fileName);
		self->fileName = NULL;
	}
	if (self->settingsFileName != NULL)
	{
		free(self->settingsFileName);
		self->settingsFileName = NULL;
	}
	if (self->tabSpaceStr != NULL)
	{
		free(self->tabSpaceStr);
		self->tabSpaceStr = NULL;
	}
	// Clear last error, just in case
	self->lastErr[0] = L'\0';
}

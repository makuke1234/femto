#include "femtoSettings.h"
#include "femtoArg.h"
#include "femto.h"

#include <jsonParser.h>

void femtoSettings_reset(femtoSettings_t * restrict self)
{
	assert(self != NULL);
	*self = (femtoSettings_t){
		.helpRequested    = false,
		.fileName         = NULL,
		.settingsFileName = NULL,

		.tabsToSpaces = false,
		.tabWidth     = 4,
		.tabSpaceStr1 = NULL,

		.autoIndent = true,
		
		.whitespaceVisible = false,
		.whitespaceCh      = L'→',
		.whitespaceCol     = FEMTO_DEFAULT_COLOR,


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
	wchar_t * mem = realloc(self->tabSpaceStr1, sizeof(wchar_t) * (self->tabWidth + 1));
	if (mem == NULL)
	{
		return false;
	}
	mem[self->tabWidth] = L'\0';
	for (uint8_t i = 0; i < self->tabWidth; ++i)
	{
		mem[i] = L' ';
	}
	self->tabSpaceStr1 = mem;

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
	femtoArg_t farg;

	femtoArg_fetchArgv(argc, argv, L"file", &mi, 1, &farg);
	if (mi != 0)
	{
		self->fileName = wcsdup_s(farg.begin, (size_t)(farg.end - farg.begin));
		if (self->fileName == NULL)
		{
			free(argumentsUsed);
			return femtoErr_memory;
		}
		argumentsUsed[mi - 1] = true;
	}

	/* **************** Other settings ******************* */

	femtoArg_fetchArgv(argc, argv, L"settings", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"setting", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		wchar_t * mem = wcsdup_s(farg.begin, (size_t)(farg.end - farg.begin));
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

	femtoArg_fetchArgv(argc, argv, L"tabsSpaces", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"tabsToSpaces", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->tabsToSpaces = femtoArg_strToBool(farg);
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"tabWidth", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"tabw", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->tabWidth = (uint8_t)u32Clamp((uint32_t)wcstoul(farg.begin, NULL, 10), 1, 32);
		if (!femtoSettings_makeTabSpaceStr(self))
		{
			free(argumentsUsed);
			return femtoErr_memory;
		}

		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"autoindent", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"autoi", &mi, 1, &farg);
	}
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"aindent", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->autoIndent = femtoArg_strToBool(farg);
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"whitespaceVisible", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"visibleWhitespace", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->whitespaceVisible = femtoArg_strToBool(farg);
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"whitespaceCh", &mi, 1, &farg);
	if (mi != 0)
	{
		self->whitespaceCh = femtoArg_strToCh(farg);
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"whitespaceColor", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"whitespaceCol", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->whitespaceCol = (uint16_t)u32Clamp((uint32_t)wcstoul(farg.begin, NULL, 10), 0, UINT16_MAX);
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
	const wchar_t * result = femtoSettings_loadFromFile(self);
	if (result != NULL)
	{
		wcsncpy_s(self->lastErr, FEMTO_SETTINGS_ERR_MAX, result, FEMTO_SETTINGS_ERR_MAX);
	}

	// Everything is OK
	free(argumentsUsed);

	return result == NULL ? femtoErr_ok : femtoErr_unknown;
}

const wchar_t * femtoSettings_loadFromFile(femtoSettings_t * restrict self)
{
	assert(self != NULL);
	if (self->settingsFileName == NULL)
	{
		return NULL;
	}
	// Generate default values
	femtoSettings_t def;
	femtoSettings_reset(&def);

	// Try to load file, for every value, change it only if it's default value
	HANDLE hset = femtoFile_sopen(self->settingsFileName, false);
	if (hset == INVALID_HANDLE_VALUE)
	{
		return L"Error opening file!";
	}

	char * bytes = NULL;
	uint32_t bytesLen = 0;
	// Read bytes
	const wchar_t * result = femtoFile_sreadBytes(hset, &bytes, &bytesLen);
	// Close file
	CloseHandle(hset);
	if (result != NULL)
	{
		if (bytes != NULL)
		{
			free(bytes);
		}
		return result;
	}

	json_t json;
	json_init(&json);

	jsonErr_t jresult = json_parse(&json, bytes, bytesLen);
	if (jresult != jsonErr_ok)
	{
		free(bytes);
		static wchar_t str[MAX_STATUS];
		swprintf_s(str, MAX_STATUS, L"%S", g_jsonErrors[jresult]);
		return str;
	}

	free(bytes);

	// Browse JSON object
	bool suc;
	jsonObject_t * obj = jsonValue_getObject(&json.value, &suc);

	if (suc)
	{
		jsonValue_t * attr = jsonObject_get(obj, "tabsToSpaces");
		if (attr != NULL)
		{
			bool value = jsonValue_getBoolean(attr, &suc);
			if (suc && (def.tabsToSpaces != value))
			{
				self->tabsToSpaces = value;
			}
		}

		attr = jsonObject_get(obj, "tabWidth");
		if (attr != NULL)
		{
			double value = jsonValue_getNumber(attr, &suc);
			uint8_t val8 = (uint8_t)value;
			if (suc && (value > 0.0) && (value < 256.0) && (def.tabWidth != val8))
			{
				self->tabWidth = val8;
			}
		}

		attr = jsonObject_get(obj, "autoindent");
		if (attr != NULL)
		{
			bool value = jsonValue_getBoolean(attr, &suc);
			if (suc && (def.autoIndent != value))
			{
				self->autoIndent = value;
			}
		}

		attr = jsonObject_get(obj, "whitespaceVisible");
		if (attr != NULL)
		{
			bool value = jsonValue_getBoolean(attr, &suc);
			if (suc && (def.whitespaceVisible != value))
			{
				self->whitespaceVisible = value;
			}
		}

		attr = jsonObject_get(obj, "whitespaceCh");
		if (attr != NULL)
		{
			const char * str = jsonValue_getString(attr, &suc);
			size_t len = strlen(str);
			if (suc && (str != NULL) && len)
			{
				char * esc = femto_escapeStr_s(str, len);
				size_t esclen = strlen(esc);
				if (esc != NULL)
				{
					if (esclen)
					{
						wchar_t * esc16 = NULL;
						uint32_t esc16len = femto_convToUnicode(esc, (int)esclen, &esc16, NULL);
						if (esc16 != NULL)
						{
							if (esc16len)
							{
								self->whitespaceCh = esc16[0];
							}
							free(esc16);
						}
					}
					free(esc);
				}
			}
		}

		attr = jsonObject_get(obj, "whitespaceColor");
		if (attr != NULL)
		{
			double value = jsonValue_getNumber(attr, &suc);
			uint16_t val = (uint16_t)value;
			if (suc && (value >= 0.0) && (value <= (double)UINT16_MAX) && val != def.whitespaceCol)
			{
				self->whitespaceCol = val;
			}
		}
	}

	// Free JSON object
	json_destroy(&json);

	return NULL;
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
	if (self->tabSpaceStr1 != NULL)
	{
		free(self->tabSpaceStr1);
		self->tabSpaceStr1 = NULL;
	}
	// Clear last error, just in case
	self->lastErr[0] = L'\0';
}

#include "femtoSettings.h"
#include "femtoArg.h"
#include "femto.h"

#include <jsonParser.h>

void femtoSettings_reset(femtoSettings_t * restrict self)
{
	assert(self != NULL);
	*self = (femtoSettings_t){
		.bHelpRequest     = false,
		.fileName         = NULL,
		.settingsFileName = NULL,

		.bTabsToSpaces = false,
		.tabWidth      = 4,
		.tabSpaceStr1  = NULL,

		.bAutoIndent = true,
		
		.bWhiteSpaceVis = false,
		.whitespaceCh   = L'→',
		.whitespaceCol  = FEMTO_DEFAULT_COLOR,

		.bRelLineNums = false,
		.lineNumCol   = FEMTO_DEFAULT_COLOR,

		.syntaxColors = {
			[tcTEXT]            = FEMTO_DEFAULT_COLOR,
			[tcCOMMENT_LINE]    = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
			[tcCOMMENT_BLOCK]   = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | BACKGROUND_INTENSITY,
			[tcKEYWORD]         = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED,
			[tcSTRING]          = FOREGROUND_RED,
			[tcCHARACTER]       = FOREGROUND_RED,
			[tcESCAPE]          = FOREGROUND_INTENSITY | FOREGROUND_RED,
			[tcSTRING_QUOTE]    = FOREGROUND_RED,
			[tcCHARACTER_QUOTE] = FOREGROUND_RED,
			[tcPREPROC]         = FOREGROUND_INTENSITY | FOREGROUND_RED,
			[tcNUMBER]          = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
			[tcHEX]             = FOREGROUND_GREEN | FOREGROUND_RED,
			[tcOCT]             = FOREGROUND_RED,
			[tcPUNCTUATION]     = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
			[tcMD_HEADING]      = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
			[tcMD_ITALIC]       = FOREGROUND_INTENSITY | FOREGROUND_RED,
			[tcMD_BOLD]         = FOREGROUND_RED | FOREGROUND_BLUE,
			[tcMD_STRIKE]       = FOREGROUND_INTENSITY,
			[tcMD_VALUE]        = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
			[tcMD_CONETEXT]     = FOREGROUND_RED | FOREGROUND_GREEN,
			[tcMD_BRACKET]      = FOREGROUND_GREEN,
			[tcMD_BRACKETPIC]   = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
			[tcMD_LINK]         = FOREGROUND_INTENSITY | FOREGROUND_BLUE
		},

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

femtoErr_e femtoSettings_populate(femtoSettings_t * restrict self, int argc, const wchar_t ** restrict argv)
{
	assert(self != NULL);
	assert(argc > 0);
	assert(argv != NULL);

	bool * argumentsUsed = calloc((size_t)(argc - 1), sizeof(bool));
	if (argumentsUsed == NULL)
	{
		return ferrMEMORY;
	}

	int mi;
	// Search for help argument first
	femtoArg_fetchArgv(argc, argv, L"help", &mi, 0);
	if (mi != 0)
	{
		self->bHelpRequest = true;
		free(argumentsUsed);
		return ferrOK;
	}
	femtoArg_t farg;

	femtoArg_fetchArgv(argc, argv, L"file", &mi, 1, &farg);
	if (mi != 0)
	{
		self->fileName = wcsdup_s(farg.begin, (size_t)(farg.end - farg.begin));
		if (self->fileName == NULL)
		{
			free(argumentsUsed);
			return ferrMEMORY;
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
			return ferrMEMORY;
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
		self->bTabsToSpaces = femtoArg_strToBool(farg);
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"tabWidth", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"tabw", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->tabWidth = (uint8_t)u32Clamp((uint32_t)wcstoul(farg.begin, NULL, 10), FEMTO_SETTINGS_MINTAB, FEMTO_SETTINGS_MAXTAB);
		if (!femtoSettings_makeTabSpaceStr(self))
		{
			free(argumentsUsed);
			return ferrMEMORY;
		}

		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"autoIndent", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"autoI", &mi, 1, &farg);
	}
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"aIndent", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->bAutoIndent = femtoArg_strToBool(farg);
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"whitespaceVisible", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"visibleWhitespace", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->bWhiteSpaceVis = femtoArg_strToBool(farg);
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
		self->whitespaceCol = (uint16_t)u32Clamp((uint32_t)wcstoul(farg.begin, NULL, 10), FEMTO_SETTINGS_MINCOLOR, FEMTO_SETTINGS_MAXCOLOR);
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"lineNumRelative", &mi, 1, &farg);
	if (mi != 0)
	{
		self->bRelLineNums = femtoArg_strToBool(farg);
		argumentsUsed[mi - 1] = true;
	}

	femtoArg_fetchArgv(argc, argv, L"lineNumColor", &mi, 1, &farg);
	if (mi == 0)
	{
		femtoArg_fetchArgv(argc, argv, L"lineNumCol", &mi, 1, &farg);
	}
	if (mi != 0)
	{
		self->lineNumCol = (uint16_t)u32Clamp((uint32_t)wcstoul(farg.begin, NULL, 10), FEMTO_SETTINGS_MINCOLOR, FEMTO_SETTINGS_MAXCOLOR);
		argumentsUsed[mi - 1] = true;
	}


	/* *************************************************** */

	// If filename wasn't specified and the last argument isn't used, take the last argument
	// If the last argument is already in use, leave fileName as NULL, opens an empty new file
	if ((self->fileName == NULL) && !argumentsUsed[argc - 2] && (argc > 1))
	{
		self->fileName = wcsdup_s(argv[argc - 1], SIZE_MAX);
		if (self->fileName == NULL)
		{
			free(argumentsUsed);
			return ferrMEMORY;
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

	return result == NULL ? ferrOK : ferrUNKNOWN;
}


#define CHECK_ERR UINT16_MAX

static inline uint16_t femtoSettings_checkColor(const jsonObject_t * restrict obj, const char * key)
{
	const jsonValue_t * attr = jsonObject_get(obj, key);
	if (attr == NULL)
	{
		return CHECK_ERR;
	}

	bool success;
	const double value = jsonValue_getNumber(attr, &success);
	if (success && (value >= (double)FEMTO_SETTINGS_MINCOLOR) && (value <= (double)FEMTO_SETTINGS_MAXCOLOR))
	{
		return (uint16_t)value;
	}
	else
	{
		return CHECK_ERR;
	}
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
	const jsonObject_t * obj = jsonValue_getObject(&json.value, &suc);

	if (suc)
	{
		const jsonValue_t * attr;
		uint16_t val;

		if ((attr = jsonObject_get(obj, "tabsToSpaces")) != NULL)
		{
			bool value = jsonValue_getBoolean(attr, &suc);
			if (suc && (def.bTabsToSpaces != value))
			{
				self->bTabsToSpaces = value;
			}
		}

		if ((attr = jsonObject_get(obj, "tabWidth")) != NULL)
		{
			const double value = jsonValue_getNumber(attr, &suc);
			const uint8_t val8 = (uint8_t)value;
			if (suc && (value >= FEMTO_SETTINGS_MINTAB) && (value <= FEMTO_SETTINGS_MAXTAB) && (def.tabWidth != val8))
			{
				self->tabWidth = val8;
			}
		}

		if ((attr = jsonObject_get(obj, "autoIndent")) != NULL)
		{
			bool value = jsonValue_getBoolean(attr, &suc);
			if (suc && (def.bAutoIndent != value))
			{
				self->bAutoIndent = value;
			}
		}

		if ((attr = jsonObject_get(obj, "whitespaceVisible")) != NULL)
		{
			bool value = jsonValue_getBoolean(attr, &suc);
			if (suc && (def.bWhiteSpaceVis != value))
			{
				self->bWhiteSpaceVis = value;
			}
		}

		if ((attr = jsonObject_get(obj, "whitespaceCh")) != NULL)
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

		if (((val = femtoSettings_checkColor(obj, "whitespaceColor")) != CHECK_ERR) && (val != def.whitespaceCol))
		{
			self->whitespaceCol = val;
		}

		if ((attr = jsonObject_get(obj, "lineNumRelative")) != NULL)
		{
			bool value = jsonValue_getBoolean(attr, &suc);
			if (suc && (def.bRelLineNums != value))
			{
				self->bRelLineNums = value;
			}
		}

		if (((val = femtoSettings_checkColor(obj, "lineNumColor")) != CHECK_ERR) && (val != def.lineNumCol))
		{
			self->lineNumCol = val;
		}

		const jsonObject_t * hObj;
		if (((attr = jsonObject_get(obj, "highlighting")) != NULL) && 
			(((hObj = jsonValue_getObject(attr, &suc)) != NULL) && suc) )
		{
			// If highlighting settings exist
			
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

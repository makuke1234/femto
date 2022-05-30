#include "femtoArg.h"

bool femtoArg_strToBool(femtoArg_t arg)
{
	if (((arg.end - arg.begin) >= 4) && (wcsncmp(arg.begin, L"true", 4) == 0))
	{
		return true;
	}
	else if (((arg.end - arg.begin) >= 5) && (wcsncmp(arg.begin, L"false", 5) == 0))
	{
		return false;
	}
	else
	{
		// String to int conversion
		return wcstol(arg.begin, NULL, 10) != 0;
	}
}
wchar femtoArg_strToCh(femtoArg_t arg)
{
	assert(arg.begin != NULL);
	assert(arg.end   != NULL);

	return ((arg.end - arg.begin) >= 1) ? arg.begin[0] : L'\0';
}


u32 femtoArg_fetch(
	const wchar * restrict rawStr, i32 maxStr,
	const wchar * restrict argMatch, u32 maxParams, ...
)
{
	assert(rawStr != NULL);
	assert(argMatch != NULL);

	va_list ap;
	va_start(ap, maxParams);

	u32 result = femtoArg_vfetch(rawStr, maxStr, argMatch, maxParams, ap);

	va_end(ap);
	return result;
}
u32 femtoArg_vfetch(
	const wchar * restrict rawStr, i32 maxStr,
	const wchar * restrict argMatch, u32 maxParams, va_list ap
)
{
	assert(rawStr != NULL);
	assert(argMatch != NULL);

	// Get real rawStr length
	u32 len = (maxStr == -1) ? (u32)wcslen(rawStr) : (u32)maxStr;

	/*
	 * Pattern
	 * --option
	 * -option
	 * /option
	 * 
	 * Values:
	 * --option=value1,value2,value3
	 * --option=value1;value2;value3
	 * --option=value1 value2 value3
	 * 
	 */
	const wchar * restrict rawIt = rawStr;
	const wchar * restrict endp = rawIt + len;
	if ((len > 1) && ((*rawIt == '-') || (*rawIt == '/')))
	{
		++rawIt;
		if ((len > 2) || (*rawIt == '-'))
		{
			++rawIt;
		}
	}
	else
	{
		return 0;
	}

	// Scan for a match
	usize matchLen = wcslen(argMatch);
	if (wcsncmp(rawIt, argMatch, matchLen) != 0)
	{
		// Didn't find a match
		return 0;
	}

	// Advance search location
	rawIt += matchLen;

	if (*rawIt == '=')
	{
		++rawIt;
		// Search for arguments
		u32 numArgs = 0;

		const wchar * restrict argStart = rawIt;

		for (; rawIt != endp; ++rawIt)
		{
			if ((*rawIt == '\\') && ((rawIt + 1) != endp))
			{
				++rawIt;
				continue;
			}
			else if ((*rawIt == L',') || (*rawIt == L';') || (*rawIt == L'?') || ((rawIt + 1) == endp))
			{
				if (numArgs < maxParams)
				{
					femtoArg_t * arg = va_arg(ap, femtoArg_t *);
					++numArgs;

					// Set argument settings
					arg->begin = argStart;
					arg->end   = rawIt + ((rawIt + 1) == endp);

					// Set new argument start position
					if ((rawIt + 1) != endp)
					{
						++rawIt;
						argStart = rawIt;
					}
				}
				else
				{
					return numArgs + 1;
				}
			}
		}

		return numArgs + 1;
	}
	else
	{
		// Argument itself counts as 1
		return 1;
	}
}


u32 femtoArg_fetchArgv(
	int argc, const wchar ** restrict argv,
	const wchar * restrict argMatch, int * restrict matchedIndex, u32 maxParams, ...
)
{
	assert(argv != NULL);
	assert(argMatch != NULL);
	assert(matchedIndex != NULL);
	
	va_list ap;
	va_start(ap, maxParams);
	
	u32 result = femtoArg_vfetchArgv(argc, argv, argMatch, matchedIndex, maxParams, ap);

	va_end(ap);
	return result;
}
u32 femtoArg_vfetchArgv(
	int argc, const wchar ** restrict argv,
	const wchar * restrict argMatch, int * restrict matchedIndex, u32 maxParams, va_list ap
)
{
	assert(argv != NULL);
	assert(argMatch != NULL);
	assert(matchedIndex != NULL);

	for (int i = 1; i < argc; ++i)
	{
		u32 result = femtoArg_vfetch(argv[i], -1, argMatch, maxParams, ap);
		if (result != 0)
		{
			*matchedIndex = i;
			return result;
		}
	}

	*matchedIndex = 0;
	return 0;
}

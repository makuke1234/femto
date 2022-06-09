#include "fArg.h"

bool fArg_strToBool(fArg_t arg)
{
	assert(arg.begin != NULL);
	assert(arg.end   != NULL);

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
wchar fArg_strToCh(fArg_t arg)
{
	assert(arg.begin != NULL);
	assert(arg.end   != NULL);

	return ((arg.end - arg.begin) >= 1) ? arg.begin[0] : L'\0';
}


usize fArg_fetch(
	const wchar * restrict rawStr, isize maxStr,
	const wchar * restrict argMatch, usize maxParams, ...
)
{
	assert(rawStr != NULL);
	assert(argMatch != NULL);

	va_list ap;
	va_start(ap, maxParams);

	const usize result = fArg_vfetch(rawStr, maxStr, argMatch, maxParams, ap);

	va_end(ap);
	return result;
}
usize fArg_vfetch(
	const wchar * restrict rawStr, isize maxStr,
	const wchar * restrict argMatch, usize maxParams, va_list ap
)
{
	assert(rawStr != NULL);
	assert(argMatch != NULL);

	// Get real rawStr length
	const usize len = (maxStr == -1) ? wcslen(rawStr) : (usize)maxStr;

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
	if ((len > 1) && ((*rawIt == L'-') || (*rawIt == L'/')))
	{
		++rawIt;
		if ((len > 2) || (*rawIt == L'-'))
		{
			++rawIt;
		}
	}
	else
	{
		return 0;
	}

	// Scan for a match
	const usize matchLen = wcslen(argMatch);
	if (wcsncmp(rawIt, argMatch, matchLen) != 0)
	{
		// Didn't find a match
		return 0;
	}

	// Advance search location
	rawIt += matchLen;

	if (*rawIt == L'=')
	{
		++rawIt;
		// Search for arguments
		usize numArgs = 0;

		const wchar * restrict argStart = rawIt;

		for (; rawIt != endp; ++rawIt)
		{
			if ((*rawIt == L'\\') && ((rawIt + 1) != endp))
			{
				++rawIt;
				continue;
			}
			else if ((*rawIt == L',') || (*rawIt == L';') || (*rawIt == L'?') || ((rawIt + 1) == endp))
			{
				if (numArgs < maxParams)
				{
					fArg_t * arg = va_arg(ap, fArg_t *);
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


usize fArg_fetchArgv(
	int argc, const wchar ** restrict argv,
	const wchar * restrict argMatch, int * restrict matchedIndex, usize maxParams, ...
)
{
	assert(argv != NULL);
	assert(argMatch != NULL);
	assert(matchedIndex != NULL);
	
	va_list ap;
	va_start(ap, maxParams);
	
	const usize result = fArg_vfetchArgv(argc, argv, argMatch, matchedIndex, maxParams, ap);

	va_end(ap);
	return result;
}
usize fArg_vfetchArgv(
	int argc, const wchar ** restrict argv,
	const wchar * restrict argMatch, int * restrict matchedIndex, usize maxParams, va_list ap
)
{
	assert(argv != NULL);
	assert(argMatch != NULL);
	assert(matchedIndex != NULL);

	for (int i = 1; i < argc; ++i)
	{
		const usize result = fArg_vfetch(argv[i], -1, argMatch, maxParams, ap);
		if (result != 0)
		{
			*matchedIndex = i;
			return result;
		}
	}

	*matchedIndex = 0;
	return 0;
}

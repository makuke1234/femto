#include "femtoArg.h"

uint32_t femtoArg_fetch(const wchar_t * restrict rawStr, int32_t maxStr, const wchar_t * restrict argMatch, uint32_t maxParams, ...)
{
	va_list ap;
	va_start(ap, maxParams);

	uint32_t result = femtoArg_vfetch(rawStr, maxStr, argMatch, maxParams, ap);

	va_end(ap);
	return result;
}
uint32_t femtoArg_vfetch(const wchar_t * restrict rawStr, int32_t maxStr, const wchar_t * restrict argMatch, uint32_t maxParams, va_list ap)
{
	assert(rawStr != NULL);
	assert(argMatch != NULL);

	// Get real rawStr length
	uint32_t len = (maxStr == -1) ? (uint32_t)wcslen(rawStr) : (uint32_t)maxStr;

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
	const wchar_t * restrict rawIt = rawStr;
	const wchar_t * restrict endp = rawIt + len;
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
	size_t matchLen = wcslen(argMatch);
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
		uint32_t numArgs = 0;

		const wchar_t * restrict argStart = rawIt;

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


uint32_t femtoArg_fetchArgv(int argc, const wchar_t ** restrict argv, const wchar_t * restrict argMatch, int * restrict matchedIndex, uint32_t maxParams, ...)
{
	va_list ap;
	va_start(ap, maxParams);
	
	uint32_t result = femtoArg_vfetchArgv(argc, argv, argMatch, matchedIndex, maxParams, ap);

	va_end(ap);
	return result;
}
uint32_t femtoArg_vfetchArgv(int argc, const wchar_t ** restrict argv, const wchar_t * restrict argMatch, int * restrict matchedIndex, uint32_t maxParams, va_list ap)
{
	assert(argv != NULL);
	assert(argMatch != NULL);
	assert(matchedIndex != NULL);

	for (int i = 1; i < argc; ++i)
	{
		uint32_t result = femtoArg_vfetch(argv[i], -1, argMatch, maxParams, ap);
		if (result != 0)
		{
			*matchedIndex = i;
			return result;
		}
	}

	*matchedIndex = 0;
	return 0;
}

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

	return 0;
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

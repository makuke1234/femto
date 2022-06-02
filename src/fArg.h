#ifndef FEMTOARG_H
#define FEMTOARG_H

#include "fCommon.h"

typedef struct fArg
{
	const wchar * begin, * end;

} fArg_t;

/**
 * @brief Convert string to boolean
 * 
 * @param arg String wrapper
 */
bool fArg_strToBool(fArg_t arg);
/**
 * @brief Convert string to UTF-16 character
 * 
 * @param arg String wrapper
 */
wchar fArg_strToCh(fArg_t arg);


/**
 * @brief Fetches individual parameters from raw command-line argument
 * 
 * @param rawStr Raw argument string, e.g argv[1]
 * @param maxStr Maximum amount of characters to scan, -1 implies the string is null-terminated
 * @param argMatch Argument to match against
 * @param maxParams Maximum number of variadic parameters to scan for
 * @param ... Variadic arguments, addresses of receiving femtoArgument_t structures
 * @return usize Number of parameters matched, argument itself counts as 1,
 * 0 if none were found or argMatch doesn't match
 */
usize fArg_fetch(
	const wchar * restrict rawStr, isize maxStr,
	const wchar * restrict argMatch, usize maxParams, ...
);
/**
 * @brief Fetches individual parameters from raw command-line argument, takes va_list
 * 
 * @param rawStr Raw argument string, e.g argv[1]
 * @param maxStr Maximum amount of characters to scan, -1 implies the string is null-terminated
 * @param argMatch Argument to match against
 * @param maxParams Maximum number of variadic parameters to scan for
 * @param ap Variadic argument list
 * @return usize Number of parameters matched, argument itself counts as 1,
 * 0 if none were found or argMatch doesn't match
 */
usize fArg_vfetch(
	const wchar * restrict rawStr, isize maxStr,
	const wchar * restrict argMatch, usize maxParams, va_list ap
);


/**
 * @brief Fetches individual parameters from argc and argv normally given to wmain()
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @param argMatch Argument to match against
 * @param matchedIndex Address of matched argument index in argv, filled by function,
 * will be set to 0, if argMatch doesn't match any arguments
 * @param maxParams Maximum number of variadic paramaters to scan for
 * @param ... Variadic arguments, addresses of receiving femtoArgument_t structures
 * @return usize Number of parameters matched, argument itself counts as 1,
 * 0 if none were found or argMatch doesn't match
 */
usize fArg_fetchArgv(
	int argc, const wchar ** restrict argv,
	const wchar * restrict argMatch, int * restrict matchedIndex, usize maxParams, ...
);
/**
 * @brief Fetches individual parameters from argc and argv normally given to wmain(), takes va_list
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @param argMatch Argument to match against
 * @param matchedIndex Address of matched argument index in argv, filled by function,
 * will be set to 0, if argMatch doesn't match any arguments, CANNOT be NULL
 * @param maxParams Maximum number of variadic paramaters to scan for
 * @param ap Variadic argument list
 * @return usize Number of parameters matched, argument itself counts as 1,
 * 0 if none were found or argMatch doesn't match
 */
usize fArg_vfetchArgv(
	int argc, const wchar ** restrict argv,
	const wchar * restrict argMatch, int * restrict mathedIndex, usize maxParams, va_list ap
);

#endif

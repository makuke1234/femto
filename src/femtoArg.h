#ifndef FEMTOARG_H
#define FEMTOARG_H

#include "common.h"

typedef struct femtoArgument
{
	const wchar_t * begin, * end;
} femtoArgument_t;

/**
 * @brief Fetches individual parameters from raw command-line argument
 * 
 * @param rawStr Raw argument string, e.g argv[1]
 * @param maxStr Maximum amount of characters to scan, -1 implies the string is null-terminated
 * @param argMatch Argument to match against
 * @param maxParams Maximum number of variadic parameters to scan for
 * @param ... Variadic arguments, addresses of receiving femtoArgument_t structures
 * @return uint32_t Number of parameters matched, 0 if none were found or argMatch doesn't match
 */
uint32_t femtoArg_fetch(const wchar_t * restrict rawStr, int32_t maxStr, const wchar_t * restrict argMatch, uint32_t maxParams, ...);
/**
 * @brief Fetches individual parameters from raw command-line argument, takes va_list
 * 
 * @param rawStr Raw argument string, e.g argv[1]
 * @param maxStr Maximum amount of characters to scan, -1 implies the string is null-terminated
 * @param argMatch Argument to match against
 * @param maxParams Maximum number of variadic parameters to scan for
 * @param ap Variadic argument list
 * @return uint32_t Number of parameters matched, 0 if none were found or argMatch doesn't match
 */
uint32_t femtoArg_vfetch(const wchar_t * restrict rawStr, int32_t maxStr, const wchar_t * restrict argMatch, uint32_t maxParams, va_list ap);

/**
 * @brief Fetches individual parameters from argc and argv normally given to wmain()
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @param argMatch Argument to match against
 * @param matchedIndex Address of matched argument index in argv, filled by function,
 * will be set to 0, if argMatch doesn't match any arguments
 * @param maxParams Maximum number of variadic paramater to scan for
 * @param ... Variadic arguments, addresses of receiving femtoArgument_t structures
 * @return uint32_t Number of parameters matched, 0 if none were found or argMatch doesn't match any arguments
 */
uint32_t femtoArg_fetchArgv(int argc, const wchar_t ** restrict argv, const wchar_t * restrict argMatch, int * restrict matchedIndex, uint32_t maxParams, ...);
/**
 * @brief Fetches individual parameters from argc and argv normally given to wmain(), takes va_list
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @param argMatch Argument to match against
 * @param matchedIndex Address of matched argument index in argv, filled by function,
 * will be set to 0, if argMatch doesn't match any arguments
 * @param maxParams Maximum number of variadic paramater to scan for
 * @param ap Variadic argument list
 * @return uint32_t Number of parameters matched, 0 if none were found or argMatch doesn't match any arguments
 */
uint32_t femtoArg_vfetchArgv(int argc, const wchar_t ** restrict argv, const wchar_t * restrict argMathc, int * restrict mathedIndex, uint32_t maxParams, va_list ap);

#endif

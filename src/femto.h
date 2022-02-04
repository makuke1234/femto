#ifndef femto_H
#define femto_H

#include "common.h"
#include "femtoData.h"

#define MAX_STATUS 256

/**
 * @brief Get value from compact boolean array
 * 
 * @param arr Pointer to array
 * @param index Item index
 * @return Item value
 */
bool boolGet(uint8_t * restrict arr, size_t index);
/**
 * @brief Put value to compact boolean array
 * 
 * @param arr Pointer to array
 * @param index Item index
 * @param value Item value
 */
void boolPut(uint8_t * restrict arr, size_t index, bool value);

int32_t i32Min(int32_t a, int32_t b);
int32_t i32Max(int32_t a, int32_t b);
uint32_t u32Min(uint32_t a, uint32_t b);
uint32_t u32Max(uint32_t a, uint32_t b);

/**
 * @brief Set exit handler data argument pointer
 * 
 * @param pdata Pointer to femtoData_t structure
 */
void femto_exitHandlerSetVars(femtoData_t * pdata);
/**
 * @brief Specialised exit handler, executed after returning from main
 * 
 */
void femto_exitHandler(void);

/**
 * @brief Gets file name from argument vector
 * 
 * @param argc Argument vector count
 * @param argv Wide-stringed argument vector
 * @return const wchar_t* File name argument
 */
const wchar_t * femto_getFileName(int argc, const wchar_t * const * const argv);
void femto_printHelp(const wchar_t * restrict app);

enum femtoError
{
	femtoErr_unknown,
	femtoErr_file,
	femtoErr_window,
	femtoErr_memory,

	femtoErr_num_of_elems
};
/**
 * @brief Prints error message to stderr, adds newline character
 * 
 * @param errCode Error code
 */
void femto_printErr(enum femtoError errCode);
/**
 * @brief Performs text editor loop tasks
 * 
 * @param pdata Pointer to femtoData_t structure
 * @return true Normal operation
 * @return false Application is ready to quit
 */
bool femto_loop(femtoData_t * restrict pdata);
/**
 * @brief Update screen buffer
 * 
 * @param peditor Pointer to femtoData_t structure
 * @param line Pointer to Y-index of current line in relation to current viewpoint, this variable is a receiver
 * @return true Only current line needs redrawing
 * @return false Whole buffer needs redrawing
 */
bool femto_updateScrbuf(femtoData_t * restrict peditor, uint32_t * line);
/**
 * @brief Update one line in screen buffer
 * 
 * @param peditor Pointer to femtoData_t structure
 * @param node Pointer to line node
 * @param line Line number to update, starting from 0
 * @return true Update only one line
 * @return false Whole screen buffer needs updating
 */
bool femto_updateScrbufLine(femtoData_t * restrict peditor, femtoLineNode_t * restrict node, uint32_t line);

/**
 * @brief Convert UTF-8 string to UTF-16 string, allocates memory only if
 * *putf16 is too small or sz == NULL
 * 
 * @param utf8 Pointer to UTF-8 character array
 * @param numBytes Maximum number of bytes to convert (including null-terminator)
 * @param putf16 Address of wchar_t pointer, the pointer itself can be initally NULL
 * @param sz Address of UTF-16 array size, can be NULL
 * @return uint32_t Number of characters converted
 */
uint32_t femto_convToUnicode(const char * restrict utf8, int numBytes, wchar_t ** restrict putf16, uint32_t * restrict sz);
/**
 * @brief Convert UTF-16 string to UTF-8 string, allocates memory only if
 * *putf8 is too small or sz == NULL
 * 
 * @param utf16 Pointer to UTF-16 character array
 * @param numChars Maximum number of bytes to convert (including null-terminator)
 * @param putf8 Address of char pointer, the pointer itself can be initally NULL
 * @param sz Address of UTF-8 array size, can be NULL
 * @return uint32_t Number of characters converted
 */
uint32_t femto_convFromUnicode(const wchar_t * restrict utf16, int numChars, char ** restrict putf8, uint32_t * restrict sz);
/**
 * @brief Convert UTF-16 string to lines array, modifies original string. After
 * creation the double-pointer's "lines" can be safely freed with a single free.
 * It's just an array of pointer pointing to parts of the original string
 * 
 * @param utf16 Pointer to UTF-16 character array
 * @param chars Maximum number of bytes to scan
 * @param lines Address of wchar_t double-pointer, double-pointer hosts wchar_t
 * pointer array, where each pointer is a character array representing line in the text file.
 * Initial value of double-pointer is irrelevant
 * @param eolSeq Address of femtoEOLsequence enumerator, receives the EOL format used
 * @return uint32_t Number of lines found
 */
uint32_t femto_strnToLines(wchar_t * restrict utf16, uint32_t chars, wchar_t *** restrict lines, enum femtoEOLsequence * restrict eolSeq);
/**
 * @brief Converts all tabs in string to spaces, modifies original string
 * 
 * @param str Address of a pointer to UTF-16 character array
 * @param len Address of string length (including null-terminator), can be NULL
 * @return uint32_t New length of converted string (including null-terminator), same as *len, if len != NULL
 */
uint32_t femto_tabsToSpaces(wchar_t ** restrict str, uint32_t * restrict len);

#endif

#ifndef FEMTO_H
#define FEMTO_H

#include "common.h"
#include "femtoData.h"
#include "femtoSyntax.h"

#define MAX_STATUS 256

enum specialAsciiCodes
{
	sacCTRL_E = 5,
	sacCTRL_N = 14,
	sacCTRL_O = 15,
	sacCTRL_Q = 17,
	sacCTRL_R = 18,
	sacCTRL_S = 19,
	sacCTRL_W = 23,

	sacLAST_CODE = 31
};


i32 min_i32(i32 a, i32 b);
i32 max_i32(i32 a, i32 b);
i32 clamp_i32(i32 value, i32 min, i32 max);

u32 min_u32(u32 a, u32 b);
u32 max_u32(u32 a, u32 b);
u32 clamp_u32(u32 value, u32 min, u32 max);

/**
 * @brief Concatenate Unicode code point at the end of the UTF-8 string, resizes if necessary,
 * stores string capacity in psize and length (including null-terminator) in plen
 * 
 * @param pstr Address of pointer to string array
 * @param psize Address of string capacity, can be NULL
 * @param plen Aadress of string length, not including null-terminator
 * @param cp 16-bit Unicode code point
 * @return char* Pointer to string array, NULL on failure
 */
char * femto_cpcat_s(char ** restrict pstr, usize * restrict psize, usize * plen, wchar cp);

/**
 * @brief Escapes the escape characters in given character array,
 * produces pointer to the new character array
 * 
 * @param inp Input null-terminated character array
 * @return char* Output character array
 */
char * femto_escapeStr(const char * inp);
/**
 * @brief Escapes the escape characters in given character array,
 * produces pointer to the new character array
 * 
 * @param inp Input character array
 * @param len Input character array length in characters
 * @return char* Output character array
 */
char * femto_escapeStr_s(const char * inp, usize len);

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
 * @brief Shows help
 * 
 * @param app Pointer to application pathname
 */
void femto_printHelp(const wchar * restrict app);
/**
 * @brief Shows clue how to get help
 * 
 * @param app Pointer to application pathname
 */
void femto_printHelpClue(const wchar * restrict app);

/**
 * @brief Asks user input on last line of editor
 * 
 * @param peditor Pointer to femtoData_t structure
 * @param line Pointer to array which will hold the result
 * @param maxLen Length of that array in characters (including null-terminator)
 * @return true User accepted and entered input
 * @return false User canceled
 */
bool femto_askInput(femtoData_t * restrict peditor, wchar * restrict line, u32 maxLen);
/**
 * @brief Performs text editor loop tasks
 * 
 * @param pdata Pointer to femtoData_t structure
 * @return true Normal operation
 * @return false Application is ready to quit
 */
bool femto_loop(femtoData_t * restrict pdata);
/**
 * @brief Performs text editor drawing asynchronously
 * 
 * @param pdata Pointer to femtoData_t structure
 * @return DWORD Irrelevant return value
 */
DWORD WINAPI femto_loopDraw(LPVOID pdata);
/**
 * @brief Tries to create a loop-drawing thread
 * 
 * @param pdata Pointer to femtoData structure
 * @return true Success
 * @return false Failure creating thread
 */
bool femto_loopDraw_createThread(femtoData_t * restrict pdata);
/**
 * @brief Closes the loopDrawing thread
 * 
 * @param pdata Pointer to femtoData structure
 */
void femto_loopDraw_closeThread(femtoData_t * restrict pdata);

/**
 * @brief Update screen buffer
 * 
 * @param peditor Pointer to femtoData_t structure
 * @param line Pointer to Y-index of current line in relation to current viewpoint, this variable is a receiver
 * @return true Only current line needs redrawing
 * @return false Whole buffer needs redrawing
 */
bool femto_updateScrbuf(femtoData_t * restrict peditor, u32 * line);
/**
 * @brief Update one line in screen buffer
 * 
 * @param peditor Pointer to femtoData_t structure
 * @param node Pointer to line node
 * @param line Line number to update, starting from 0
 * @return true Update only one line
 * @return false Whole screen buffer needs updating
 */
bool femto_updateScrbufLine(femtoData_t * restrict peditor, struct femtoLineNode * restrict node, u32 line);

/**
 * @brief Convert UTF-8 string to UTF-16 string, allocates memory only if
 * *putf16 is too small or sz == NULL
 * 
 * @param utf8 Pointer to UTF-8 character array
 * @param numBytes Maximum number of bytes to convert (including null-terminator)
 * @param putf16 Address of wchar pointer, the pointer itself can be initally NULL
 * @param sz Address of UTF-16 array size, can be NULL
 * @return u32 Number of characters converted
 */
u32 femto_convToUnicode(const char * restrict utf8, int numBytes, wchar ** restrict putf16, u32 * restrict sz);
/**
 * @brief Convert UTF-16 string to UTF-8 string, allocates memory only if
 * *putf8 is too small or sz == NULL
 * 
 * @param utf16 Pointer to UTF-16 character array
 * @param numChars Maximum number of bytes to convert (including null-terminator)
 * @param putf8 Address of char pointer, the pointer itself can be initally NULL
 * @param sz Address of UTF-8 array size, can be NULL
 * @return u32 Number of characters converted
 */
u32 femto_convFromUnicode(const wchar * restrict utf16, int numChars, char ** restrict putf8, u32 * restrict sz);
/**
 * @brief Convert UTF-16 string to lines array, modifies original string. After
 * creation the double-pointer's "lines" can be safely freed with a single free.
 * It's just an array of pointer pointing to parts of the original string
 * 
 * @param utf16 Pointer to UTF-16 character array
 * @param chars Maximum number of bytes to scan
 * @param lines Address of wchar double-pointer, double-pointer hosts wchar
 * pointer array, where each pointer is a character array representing line in the text file.
 * Initial value of double-pointer is irrelevant
 * @param eolSeq Address of eolSequence enumerator, receives the EOL format used
 * @return u32 Number of lines found
 */
u32 femto_strnToLines(wchar * restrict utf16, u32 chars, wchar *** restrict lines, enum eolSequence * restrict eolSeq);

/**
 * @brief Tests if file with designated filename exists
 * 
 * @param filename Pointer to filename widestring
 * @return true File exists
 * @return false File doesn't exist/is inaccessible
 */
bool femto_testFile(const wchar * filename);

#endif

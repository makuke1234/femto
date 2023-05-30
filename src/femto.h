#ifndef FEMTO_H
#define FEMTO_H

#include "fCommon.h"
#include "fData.h"
#include "fSyntax.h"

typedef enum specialASCIIcodes
{
	sacCTRL_A = 1,
	sacCTRL_B = 2,
	sacCTRL_C = 3,
	sacCTRL_D = 4,
	sacCTRL_E = 5,
	sacCTRL_F = 6,
	sacCTRL_G = 7,
	sacCTRL_H = 8,
	sacCTRL_I = 9,
	sacCTRL_J = 10,
	sacCTRL_K = 11,
	sacCTRL_L = 12,
	sacCTRL_M = 13,
	sacCTRL_N = 14,
	sacCTRL_O = 15,
	sacCTRL_P = 16,
	sacCTRL_Q = 17,
	sacCTRL_R = 18,
	sacCTRL_S = 19,
	sacCTRL_T = 20,
	sacCTRL_U = 21,
	sacCTRL_V = 22,
	sacCTRL_W = 23,
	sacCTRL_X = 24,
	sacCTRL_Y = 25,
	sacCTRL_Z = 26,

	sacLAST_CODE = 31

} specialASCIIcodes_e, sac_e;


i32 min_i32(i32 a, i32 b);
i32 max_i32(i32 a, i32 b);
i32 clamp_i32(i32 value, i32 min, i32 max);
i32 clampdc_i32(i32 value, i32 range1, i32 range2);

u32 min_u32(u32 a, u32 b);
u32 max_u32(u32 a, u32 b);
u32 clamp_u32(u32 value, u32 min, u32 max);
u32 clampdc_u32(u32 value, u32 range1, u32 range2);

i64 min_i64(i64 a, i64 b);
i64 max_i64(i64 a, i64 b);
i64 clamp_i64(i64 value, i64 min, i64 max);
i64 clampdc_i64(i64 value, i64 range1, i64 range2);

u64 min_u64(u64 a, u64 b);
u64 max_u64(u64 a, u64 b);
u64 clamp_u64(u64 value, u64 min, u64 max);
u64 clampdc_u64(u64 value, u64 range1, u64 range2);

usize min_usize(usize a, usize b);
usize max_usize(usize a, usize b);
usize clamp_usize(usize value, usize min, usize max);
usize clampdc_usize(usize value, usize range1, usize range2);



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
char * femto_escStr(const char * restrict inp);
/**
 * @brief Escapes the escape characters in given character array,
 * produces pointer to the new character array
 * 
 * @param inp Input character array
 * @param len Input character array length in characters
 * @return char* Output character array
 */
char * femto_escStr_s(const char * restrict inp, usize len);

/**
 * @brief Set exit handler data argument pointer
 * 
 * @param pdata Pointer to fData_t structure
 */
void femto_exitHandlerSetVars(fData_t * restrict pdata);
/**
 * @brief Specialised exit handler, executed after returning from main
 * 
 */
void femto_exitHandler(void);

/**
 * @brief Shows help
 * 
 * @param app Pointer to application pathname
 * @param helpArg Pointer to help search keyword, can be NULL
 */
void femto_printHelp(const wchar * restrict app, const wchar * restrict helpArg);
/**
 * @brief Shows clue how to get help
 * 
 * @param app Pointer to application pathname
 */
void femto_printHelpClue(const wchar * restrict app);

/**
 * @brief Asks user input on last line of editor
 * 
 * @param peditor Pointer to fData_t structure
 * @param line Pointer to array which will hold the result
 * @param maxLen Length of that array in characters (including null-terminator)
 * @return true User accepted and entered input
 * @return false User canceled
 */
bool femto_askInput(fData_t * restrict peditor, wchar * restrict line, u32 maxLen);
/**
 * @brief Performs text editor loop tasks
 * 
 * @param pdata Pointer to fData_t structure
 * @return true Normal operation
 * @return false Application is ready to quit
 */
bool femto_loop(fData_t * restrict pdata);
/**
 * @brief Performs text editor drawing asynchronously
 * 
 * @param pdata Pointer to fData_t structure
 * @return DWORD Irrelevant return value
 */
DWORD WINAPI femto_asyncDraw(LPVOID pdata);
/**
 * @brief Tries to create a loop-drawing thread
 * 
 * @param pdata Pointer to fData structure
 * @return true Success
 * @return false Failure creating thread
 */
bool femto_asyncDrawInit(fData_t * restrict pdata);
/**
 * @brief Closes the loopDrawing thread
 * 
 * @param pdata Pointer to fData structure
 */
void femto_asyncDrawStop(fData_t * restrict pdata);

/**
 * @brief Update screen buffer
 * 
 * @param peditor Pointer to fData_t structure
 * @param line Pointer to Y-index of current line in relation to current viewpoint, this variable is a receiver
 * @return true Only current line needs redrawing
 * @return false Whole buffer needs redrawing
 */
bool femto_updateScrbuf(fData_t * restrict peditor, u32 * restrict line);
/**
 * @brief Update one line in screen buffer
 * 
 * @param peditor Pointer to fData_t structure
 * @param node Pointer to line node
 * @param line Line number to update, starting from 0
 * @return true Update only one line
 * @return false Whole screen buffer needs updating
 */
bool femto_updateScrbufLine(fData_t * restrict peditor, struct fLine * restrict node, u32 line);

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
u32 femto_toutf16(const char * restrict utf8, int numBytes, wchar ** restrict putf16, usize * restrict sz);
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
u32 femto_toutf8(const wchar * restrict utf16, int numChars, char ** restrict putf8, usize * restrict sz);
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
 * @return usize Number of lines found
 */
usize femto_strnToLines(wchar * restrict utf16, usize chars, wchar *** restrict lines, eolSeq_e * restrict eolSeq);

/**
 * @brief Tests if file with designated filename exists
 * 
 * @param filename Pointer to filename widestring
 * @return true File exists
 * @return false File doesn't exist/is inaccessible
 */
bool femto_testFile(const wchar * restrict filename);
/**
 * @brief Retrieves the size of the desired filename
 * 
 * @param hfile Open file handle
 * @return isize File size in bytes, -1 on error of some kind
 */
isize femto_fileSize(HANDLE hfile);
/**
 * @brief Opens new file with desired name and write access
 * 
 * @param fileName Desired file name, CANNOT be NULL
 * @param writemode Desired write access, true for write mode, false for read mode
 * @return HANDLE Opened file handle, INVALID_HANDLE_VALUE on failure; Use
 * CloseHandle() to close the file afterwards
 */
HANDLE femto_openFile(const wchar * restrict fileName, bool writemode);
/**
 * @brief Reads bytes to an array from open file, allocates memory only if *bytes
 * is too small
 * 
 * @param hfile Handle to open file 
 * @param bytes Address of pointer to character array
 * @param bytesLen Address of array length in bytes
 * @return const wchar* Error message, NULL on success
 */
const wchar * femto_readBytes(HANDLE hfile, char ** restrict bytes, usize * restrict bytesLen);

/**
 * @brief Set console title according to last given filename, also shows
 * editor name on the titlebar
 * 
 * @param fileName File name
 */
void femto_setConTitle(const wchar * restrict fileName);

#endif

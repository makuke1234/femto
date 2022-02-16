#ifndef femtoFILE_H
#define femtoFILE_H

#include "common.h"
#include "femtoLine.h"
#include "femtoSettings.h"

enum femtoEOLsequence
{
	EOL_not  = 0x00,
	EOL_CR   = 0x01,
	EOL_LF   = 0x02,
	EOL_CRLF = EOL_CR | EOL_LF,

	EOL_def  = EOL_CRLF,
};

typedef struct femtoFile
{
	const wchar_t * fileName;
	HANDLE hFile;
	bool canWrite;
	enum femtoEOLsequence eolSeq;

	struct
	{
		femtoLineNode_t * firstNode;
		femtoLineNode_t * currentNode;
		femtoLineNode_t * pcury;
		uint32_t curx, lastx;

		bool typed;
		bool updateAll;
	} data;
} femtoFile_t;

/**
 * @brief Resets femtoFile_t structure memory layout, zeroes all members
 * 
 * @param self Pointer to femtoFile_t structure
 */
void femtoFile_reset(femtoFile_t * restrict self);
/**
 * @brief Allocates dynamicaly memory for femtoFile_t structure and resets its memory layout
 * 
 * @return femtoFile_t* Pointer to the newly allocated structure
 */
femtoFile_t * femtoFile_resetDyn(void);
/**
 * @brief Opens new file with desired name and write access
 * 
 * @param fileName Desired file name, CANNOT be NULL
 * @param writemode Desired write access, true for write mode, false for read mode
 * @return HANDLE Opened file handle, INVALID_HANDLE_VALUE on failure; Use
 * CloseHandle() to close the file afterwards
 */
HANDLE femtoFile_sopen(const wchar_t * restrict fileName, bool writemode);
/**
 * @brief Opens new file with desired name and write access
 * 
 * @param self Pointer to femtoFile_t structure
 * @param selfName Desired file name, can be NULL; If is NULL, previously given file
 * name will be used
 * @param writemode Desired write access, true for write mode, false for read mode
 * @return true Success
 * @return false Failure
 */
bool femtoFile_open(femtoFile_t * restrict self, const wchar_t * restrict selfName, bool writemode);
/**
 * @brief Closes open file if possible
 * 
 * @param self Pointer to femtoFile_t structure
 */
void femtoFile_close(femtoFile_t * restrict self);
/**
 * @brief Clears (internal) lines in editor data structure
 * 
 * @param self Pointer to femtoFile_t structure
 */
void femtoFile_clearLines(femtoFile_t * restrict self);
/**
 * @brief Reads bytes to an array from open file, allocates memory only if *bytes
 * is too small
 * 
 * @param hfile Handle to open file 
 * @param bytes Address of pointer to character array
 * @param bytesLen Address of array length in bytes
 * @return const wchar_t* Error messafe, NULL on success
 */
const wchar_t * femtoFile_sreadBytes(HANDLE hfile, char ** restrict bytes, uint32_t * restrict bytesLen);
/**
 * @brief Opens file with last given filename, reads bytes to an array, allocates
 * memory only if *bytes is too small
 * 
 * @param self Pointer to femtoFile_t structure
 * @param bytes Address of pointer to character array
 * @param bytesLen Address of array length in bytes
 * @return const wchar_t* Error message, NULL on success
 */
const wchar_t * femtoFile_readBytes(femtoFile_t * restrict self, char ** restrict bytes, uint32_t * restrict bytesLen);
/**
 * @brief Opens file with last given filename, reads file contents to internal
 * structure, ready to be shown on screen
 * 
 * @param self Pointer to femtoFile_t structure
 * @return const wchar_t* Error message, NULL on success
 */
const wchar_t * femtoFile_read(femtoFile_t * restrict self);

enum femtoFile_writeRes
{
	writeRes_nothingNew = -1,
	writeRes_openError  = -2,
	writeRes_writeError = -3,
	writeRes_memError   = -4
};
/**
 * @brief Opens file with last given filename, reads file contents to temporary memory,
 * compares them with current internal file contents in memory. Only attempts to write to
 * file if anything has been changed
 * 
 * @param self Pointer to femtoFile_t structure
 * @param tabWidth Tab character width in monospace characters
 * @return int32_t Negative values represent error code, positive values (0 inclusive)
 * represent number of bytes written to disc
 */
int32_t femtoFile_write(femtoFile_t * restrict self, uint8_t tabWidth);
/**
 * @brief Set console title according to last given filename, also shows
 * editor name on the titlebar
 * 
 * @param self Pointer to femtoFile_t structure
 */
void femtoFile_setConTitle(const femtoFile_t * restrict self);

/**
 * @brief Inserts a normal character to current line
 * 
 * @param self Pointer to femtoFile_t structure
 * @param ch Character to insert
 * @return true Success
 * @return false Failure
 */
bool femtoFile_addNormalCh(femtoFile_t * restrict self, wchar_t ch);
/**
 * @brief Inserts a special character to current line
 * 
 * @param self Pointer to femtoFile_t structure
 * @param height Screenbuffer height in lines
 * @param ch Character to insert
 * @param pset Pointer to femtoSettings_t structure
 * @return true Success
 * @return false Failure
 */
bool femtoFile_addSpecialCh(femtoFile_t * restrict self, uint32_t height, wchar_t ch, const femtoSettings_t * pset);

/**
 * @brief Checks current line contents for matching string
 * 
 * @param self Pointer to femtoFile_t structure
 * @param maxdelta Offset from current cursor position, value is clamped
 * @param string Pointer to character array to match with
 * @param maxString Absolute maximum number of characters to check, stops anyway on null-terminator
 * @return true Found a match
 * @return false Didn't find any match
 */
bool femtoFile_checkLineAt(const femtoFile_t * restrict self, int32_t maxdelta, const wchar_t * restrict string, uint32_t maxString);
/**
 * @brief Deletes a character on current line going forward (right, cursor stationary),
 * merges with the next line (if possible), if cursor is already at the end of the line
 * 
 * @param self Pointer to femtoFile_t structure
 * @return true Success
 * @return false Failure
 */
bool femtoFile_deleteForward(femtoFile_t * restrict self);
/**
 * @brief Deletes a character on current line going backwards (left, cursor also move to the left),
 * merges with and move to the the previous line (if possible), if cursor is
 * already at the beginning of the line
 * 
 * @param self Pointer to femtoFile_t structure
 * @return true Success
 * @return false Failure
 */
bool femtoFile_deleteBackward(femtoFile_t * restrict self);
/**
 * @brief Adds a new line after current active line
 * 
 * @param self Pointer to femtoFile_t structure
 * @param tabsToSpaces Tabs are converted to spaces
 * @param autoIndent Automatically insert tabs to match whitespace of the previous line
 * @return true Success
 * @return false Failure
 */
bool femtoFile_addNewLine(femtoFile_t * restrict self, bool tabsToSpaces, bool autoIndent);
/**
 * @brief Updates current viewpoint if necessary
 * 
 * @param self Pointer to femtoFile_t structure
 * @param height Editor window height
 */
void femtoFile_updateCury(femtoFile_t * restrict self, uint32_t height);
/**
 * @brief Scrolls current viewpoint if possible, updates viewpoint if necessary
 * 
 * @param self Pointer to femtoFile_t structure
 * @param height Editor window height
 * @param deltaLines Lines to scroll, positive values mean scrolling down, negative values mean scrolling up
 */
void femtoFile_scroll(femtoFile_t * restrict self, uint32_t height, int32_t deltaLines);
/**
 * @brief Scrolls current viewpoint if possible, updates viewpoint if necessary
 * 
 * @param self Pointer to femtoFile_t structure
 * @param width Editor window width
 * @param deltaCh Characters to scroll, positive values mean scrolling right, negative values mean scrolling up
 */
void femtoFile_scrollHor(femtoFile_t * restrict self, uint32_t width, int32_t deltaCh);


/**
 * @brief Destroys femtoFile_t structure
 * 
 * @param self Pointer to femtoFile_t structure
 */
void femtoFile_destroy(femtoFile_t * restrict self);


#endif

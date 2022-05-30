#ifndef FEMTO_FILE_H
#define FEMTO_FILE_H

#include "fCommon.h"
#include "fLine.h"
#include "fSettings.h"
#include "fSyntax.h"

#define MAX_SUFFIX 8

enum eolSequence
{
	eolNOT  = 0x00,
	eolCR   = 0x01,
	eolLF   = 0x02,
	eolCRLF = eolCR | eolLF,

	eolDEF  = eolCRLF,
};

typedef struct femtoFile
{
	wchar * fileName;
	HANDLE hFile;

	struct
	{
		femtoLineNode_t * firstNode;
		femtoLineNode_t * currentNode;
		femtoLineNode_t * pcury;
		u32 curx, lastx;

		uint8_t noLen;

		bool bTyped:1;
		bool bUpdateAll:1;
	} data;

	enum eolSequence eolSeq;
	bool bCanWrite:1;
	bool bUnsaved:1;
	bool bSyntaxByUser:1;
	enum femtoSyntax syntax:4;

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
HANDLE femtoFile_sopen(const wchar * restrict fileName, bool writemode);
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
bool femtoFile_open(femtoFile_t * restrict self, const wchar * restrict selfName, bool writemode);
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
 * @return const wchar* Error messafe, NULL on success
 */
const wchar * femtoFile_sreadBytes(HANDLE hfile, char ** restrict bytes, u32 * restrict bytesLen);
/**
 * @brief Opens file with last given filename, reads bytes to an array, allocates
 * memory only if *bytes is too small
 * 
 * @param self Pointer to femtoFile_t structure
 * @param bytes Address of pointer to character array
 * @param bytesLen Address of array length in bytes
 * @return const wchar* Error message, NULL on success
 */
const wchar * femtoFile_readBytes(femtoFile_t * restrict self, char ** restrict bytes, u32 * restrict bytesLen);
/**
 * @brief Opens file with last given filename, reads file contents to internal
 * structure, ready to be shown on screen
 * 
 * @param self Pointer to femtoFile_t structure
 * @return const wchar* Error message, NULL on success
 */
const wchar * femtoFile_read(femtoFile_t * restrict self);

enum femtoFile_checkRes
{
	checkRes_nothingNew,
	checkRes_needsSaving,
	checkRes_memError
};
/**
 * @brief Checks if any changes have been made to file, sets unsaved member of structure
 * 
 * @param self Pointer to femtoFile_t structure
 * @param editorContents Address of pointer receiving data from editor's screen, can be NULL
 * @param editorContLen Address of variable receiving size of editor's screen data, can be NULL
 * @return i32 Error code
 */
i32 femtoFile_checkUnsaved(femtoFile_t * restrict self, char ** editorContents, u32 * editorContLen);

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
 * @return i32 Negative values represent error code, positive values (0 inclusive)
 * represent number of bytes written to disc
 */
i32 femtoFile_write(femtoFile_t * restrict self);
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
 * @param tabWidth Tab character width in space equivalents
 * @return true Success
 * @return false Failure
 */
bool femtoFile_addNormalCh(femtoFile_t * restrict self, wchar ch, u32 tabWidth);
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
bool femtoFile_addSpecialCh(femtoFile_t * restrict self, u32 height, wchar ch, const femtoSettings_t * pset);

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
 * @param tabWidth Tab character width in equivalent spaces
 * @param autoIndent Automatically insert tabs to match whitespace of the previous line
 * @return true Success
 * @return false Failure
 */
bool femtoFile_addNewLine(femtoFile_t * restrict self, bool tabsToSpaces, uint8_t tabWidth, bool autoIndent);
/**
 * @brief Updates current viewpoint if necessary
 * 
 * @param self Pointer to femtoFile_t structure
 * @param height Editor window height
 */
void femtoFile_updateCury(femtoFile_t * restrict self, u32 height);
/**
 * @brief Scrolls current viewpoint if possible, updates viewpoint if necessary
 * 
 * @param self Pointer to femtoFile_t structure
 * @param height Editor window height
 * @param deltaLines Lines to scroll, positive values mean scrolling down, negative values mean scrolling up
 */
void femtoFile_scroll(femtoFile_t * restrict self, u32 height, i32 deltaLines);
/**
 * @brief Scrolls current viewpoint if possible, updates viewpoint if necessary
 * 
 * @param self Pointer to femtoFile_t structure
 * @param width Editor window width
 * @param deltaCh Characters to scroll, positive values mean scrolling right, negative values mean scrolling left
 */
void femtoFile_scrollHor(femtoFile_t * restrict self, u32 width, i32 deltaCh);


/**
 * @brief Destroys femtoFile_t structure
 * 
 * @param self Pointer to femtoFile_t structure
 */
void femtoFile_destroy(femtoFile_t * restrict self);


#endif
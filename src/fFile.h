#ifndef FEMTO_FILE_H
#define FEMTO_FILE_H

#include "fCommon.h"
#include "fLine.h"
#include "fSettings.h"
#include "fSyntax.h"

#define MAX_SUFFIX 8

typedef enum eolSequence
{
	eolNOT  = 0x00,
	eolCR   = 0x01,
	eolLF   = 0x02,
	eolCRLF = eolCR | eolLF,

	eolDEF  = eolCRLF

} eolSequence_e, eolSeq_e;

#if SSIZE_MAX == INT64_MAX
	#define MAX_USIZE_BITS_1 63
#else
	#define MAX_USIZE_BITS_1 31
#endif

typedef struct fFile
{
	wchar * fileName;
	HANDLE hFile;

	struct fFileData
	{
		fLine_t * firstNode;
		fLine_t * currentNode;
		fLine_t * pcury;
		usize curx, lastx;

		u8 noLen;

		// Highlighting info
		struct fFileHighLight
		{
			// Store pointer to highlighting beginning line, when highlighting
			// the current cursor always moves, so that is stored in currentNode
			// If no highlighting is present, the value should be NULL
			const fLine_t * beg;
			// Also store the beginning cursor of beginning line, if the index
			// is after beg->curx, beg->freeSpaceLen will be subtracted
			usize begx:MAX_USIZE_BITS_1;
			// Indicates whether the highlighting is backwards or forwards
			usize backwards:1;
		} hl;

		bool bTyped:1;
		bool bUpdateAll:1;
	} data;

	eolSeq_e eolSeq;
	bool bExists:1;
	bool bCanWrite:1;
	bool bUnsaved:1;
	bool bSyntaxByUser:1;
	fStx_e syntax:4;

} fFile_t;

#undef MAX_USIZE_BITS_1

/**
 * @brief Resets fFile_t structure memory layout, zeroes all members
 * 
 * @param self Pointer to fFile_t structure
 */
void fFile_reset(fFile_t * restrict self);
/**
 * @brief Allocates dynamicaly memory for fFile_t structure and resets its memory layout
 * 
 * @return fFile_t* Pointer to the newly allocated structure
 */
fFile_t * fFile_resetDyn(void);
/**
 * @brief Opens new file with desired name and write access
 * 
 * @param self Pointer to fFile_t structure
 * @param selfName Desired file name, can be NULL; If is NULL, previously given file
 * name will be used
 * @param writemode Desired write access, true for write mode, false for read mode
 * @return true Success
 * @return false Failure
 */
bool fFile_open(fFile_t * restrict self, const wchar * restrict selfName, bool writemode);
/**
 * @brief Closes open file if possible
 * 
 * @param self Pointer to fFile_t structure
 */
void fFile_close(fFile_t * restrict self);
/**
 * @brief Clears (internal) lines in editor data structure
 * 
 * @param self Pointer to fFile_t structure
 */
void fFile_clearLines(fFile_t * restrict self);
/**
 * @brief Opens file with last given filename, reads bytes to an array, allocates
 * memory only if *bytes is too small
 * 
 * @param self Pointer to fFile_t structure
 * @param bytes Address of pointer to character array
 * @param bytesLen Address of array length in bytes
 * @return const wchar* Error message, NULL on success
 */
const wchar * fFile_readBytes(fFile_t * restrict self, char ** restrict bytes, usize * restrict bytesLen);
/**
 * @brief Opens file with last given filename, reads file contents to internal
 * structure, ready to be shown on screen
 * 
 * @param self Pointer to fFile_t structure
 * @return const wchar* Error message, NULL on success
 */
const wchar * fFile_read(fFile_t * restrict self);

typedef enum fFile_checkRes
{
	ffcrNOTHING_NEW,
	ffcrNEEDS_SAVING,
	ffcrMEM_ERROR

} fFile_checkRes_e, fFile_cr_e, ffcr_e;
/**
 * @brief Checks if any changes have been made to file, sets unsaved member of structure
 * 
 * @param self Pointer to fFile_t structure
 * @param editorContents Address of pointer receiving data from editor's screen, can be NULL
 * @param editorContLen Address of variable receiving size of editor's screen data, can be NULL
 * @return ffcr_e Error code
 */
ffcr_e fFile_checkUnsaved(fFile_t * restrict self, char ** editorContents, usize * editorContLen);

typedef enum fFile_writeRes
{
	ffwrNOTHING_NEW = -1,
	ffwrOPEN_ERROR  = -2,
	ffwrWRITE_ERROR = -3,
	ffwrMEM_ERROR   = -4

} fFile_writeRes_e, fFile_wr_e, ffwr_e;
/**
 * @brief Opens file with last given filename, reads file contents to temporary memory,
 * compares them with current internal file contents in memory. Only attempts to write to
 * file if anything has been changed
 * 
 * @param self Pointer to fFile_t structure
 * @return isize Negative values represent error code, positive values (0 inclusive)
 * represent number of bytes written to disc
 */
isize fFile_write(fFile_t * restrict self);

/**
 * @brief Inserts a normal character to current line
 * 
 * @param self Pointer to fFile_t structure
 * @param ch Character to insert
 * @param tabWidth Tab character width in space equivalents
 * @return true Success
 * @return false Failure
 */
bool fFile_addNormalCh(fFile_t * restrict self, wchar ch, u8 tabWidth);
/**
 * @brief Handles highlighting starting/stopping depending on typed character
 * 
 * @param self Pointer to fFile_t structure
 * @param ch Character inserted
 * @param shift Determines whether the shift key is pressed down
 * @return true Highlighting is turned on
 * @return false Highlighting is turned off
 */
bool fFile_startHighlighting(fFile_t * restrict self, wchar ch, bool shift);
/**
 * @brief Inserts a special character to current line
 * 
 * @param self Pointer to fFile_t structure
 * @param height Screenbuffer height in lines
 * @param ch Character to insert
 * @param shift Determines whether the shift key is pressed down
 * @param pset Pointer to fSettings_t structure
 * @return true Success
 * @return false Failure
 */
bool fFile_addSpecialCh(
	fFile_t * restrict self, u32 height,
	wchar ch, bool shift,
	const fSettings_t * pset
);

/**
 * @brief Deletes a character on current line going forward (right, cursor stationary),
 * merges with the next line (if possible), if cursor is already at the end of the line
 * 
 * @param self Pointer to fFile_t structure
 * @return true Success
 * @return false Failure
 */
bool fFile_deleteForward(fFile_t * restrict self);
/**
 * @brief Deletes a character on current line going backwards (left, cursor also move to the left),
 * merges with and move to the the previous line (if possible), if cursor is
 * already at the beginning of the line
 * 
 * @param self Pointer to fFile_t structure
 * @return true Success
 * @return false Failure
 */
bool fFile_deleteBackward(fFile_t * restrict self);
/**
 * @brief Adds a new line after current active line
 * 
 * @param self Pointer to fFile_t structure
 * @param tabsToSpaces Tabs are converted to spaces
 * @param tabWidth Tab character width in equivalent spaces
 * @param autoIndent Automatically insert tabs to match whitespace of the previous line
 * @return true Success
 * @return false Failure
 */
bool fFile_addNewLine(fFile_t * restrict self, bool tabsToSpaces, u8 tabWidth, bool autoIndent);
/**
 * @brief Updates current viewpoint if necessary
 * 
 * @param self Pointer to fFile_t structure
 * @param height Editor window height
 */
void fFile_updateCury(fFile_t * restrict self, u32 height);
/**
 * @brief Scrolls current viewpoint if possible, updates viewpoint if necessary
 * 
 * @param self Pointer to fFile_t structure
 * @param height Editor window height
 * @param deltaLines Lines to scroll, positive values mean scrolling down, negative values mean scrolling up
 */
void fFile_scrollVert(fFile_t * restrict self, u32 height, isize deltaLines);
/**
 * @brief Scrolls current viewpoint if possible, updates viewpoint if necessary
 * 
 * @param self Pointer to fFile_t structure
 * @param width Editor window width
 * @param deltaCh Characters to scroll, positive values mean scrolling right, negative values mean scrolling left
 */
void fFile_scrollHor(fFile_t * restrict self, u32 width, isize deltaCh);


/**
 * @brief Destroys fFile_t structure
 * 
 * @param self Pointer to fFile_t structure
 */
void fFile_destroy(fFile_t * restrict self);
/**
 * @brief Frees heap-allocated fFile_t structure
 * 
 * @param self Pointer to dynamically allocated fFile_t structure
 */
void fFile_free(fFile_t * restrict self);


#endif

#ifndef femtoFILE_H
#define femtoFILE_H

#include "common.h"

#define femto_LNODE_DEFAULT_FREE 10

/*
	Example:
	L"This is text\0\0\0\0\0\0"
	              ^ - curx
				  <----------> - freeSpaceLen = 6
*/

typedef struct femtoLineNode_t
{
	wchar_t * line;
	uint32_t lineEndx, curx, freeSpaceLen;
	struct femtoLineNode_t * prevNode, * nextNode;
} femtoLineNode_t;

/**
 * @brief Creates new line in-between current line and next line
 * 
 * @param curnode Pointer to current line node, can be NULL
 * @param nextnode Pointer to next line node, can be NULL
 * @return femtoLineNode_t* Pointer to newly created line node, NULL on failure
 */
femtoLineNode_t * femtoLine_create(femtoLineNode_t * restrict curnode, femtoLineNode_t * restrict nextnode);
/**
 * @brief Creates new line in-between current line and next line
 * 
 * @param curnode Pointer to current line node, can be NULL
 * @param nextnode Pointer to next line node, can be NULL
 * @param lineText Pointer to UTF-16 character array, contents of this will be
 * copied to the newly created line
 * @param maxText Maximum amount of characters to copy (not including null-terminator),
 * can be -1, if string is null-terminated
 * @return femtoLineNode_t* Pointer to newly created line node, NULL on failure
 */
femtoLineNode_t * femtoLine_createText(
	femtoLineNode_t * restrict curnode,
	femtoLineNode_t * restrict nextnode,
	const wchar_t * restrict lineText,
	int32_t maxText
);

/**
 * @brief Fetches text from given line node, copies it to wchar_t character array,
 * allocates memory only if *text is too small or tarrsz == NULL
 * 
 * @param self Pointer to line node to fetch text from
 * @param text Address of wchar_t pointer to character array, wchar_t pointer
 * itself can be initally NULL
 * @param tarrsz Size of receiving character array, can be NULL
 * @return true Success
 * @return false Failure
 */
bool femtoLine_getText(const femtoLineNode_t * restrict self, wchar_t ** restrict text, uint32_t * restrict tarrsz);
/**
 * @brief Reallocates free space on given line node, guarantees
 * femto_LNODE_DEFAULT_FREE characters for space
 * 
 * @param self Pointer to line node
 * @return true Success
 * @return false Failure
 */
bool femtoLine_realloc(femtoLineNode_t * restrict self);

/**
 * @brief Merges current line node with next line node, adjusts current
 * y-position line node pointer if necessary
 * 
 * @param self Pointer to current line node
 * @param ppcury Address of pointer to current y-position line node
 * @return true Success
 * @return false Failure
 */
bool femtoLine_mergeNext(femtoLineNode_t * restrict self, femtoLineNode_t ** restrict ppcury);

/**
 * @brief Moves (internal) cursor on current line node, clamps movement
 * 
 * @param self Pointer to current line node
 * @param delta Amount of characters to move, positive values to move right,
 * negative values to move left
 */
void femtoLine_moveCursor(femtoLineNode_t * restrict self, int32_t delta);
/**
 * @brief Moves (internal) cursor on current line node, clamps movement
 * 
 * @param self Pointer to current line node
 * @param curx Absolute cursor position to move to
 */
void femtoLine_moveCursorAbs(femtoLineNode_t * restrict self, uint32_t curx);
/**
 * @brief Moves active line node, clamps movement
 * 
 * @param self Address of pointer to current line node
 * @param delta Amount of lines to move, positive values move down, negative values move up
 */
void femtoLine_moveCursorVert(femtoLineNode_t ** restrict self, int32_t delta);

/**
 * @brief Swaps internally data between 2 nodes
 * 
 * @param node1 First node
 * @param node2 Second node
 */
void femtoLine_swap(femtoLineNode_t * restrict node1, femtoLineNode_t * restrict node2);

/**
 * @brief Destroys line node, frees memory
 * 
 * @param self Pointer to line node
 */
void femtoLine_destroy(femtoLineNode_t * restrict self);

enum femtoEOLsequence
{
	EOL_not  = 0x00,
	EOL_CR   = 0x01,
	EOL_LF   = 0x02,
	EOL_CRLF = EOL_CR | EOL_LF,

	EOL_def  = EOL_CRLF,
};

typedef struct femtoFile_t
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
 * @param self Pointer to femtoFile_t structure
 * @param selfName Desired file name, can be NULL; If is NULL, previously given file
 * name will be used
 * @param writemode Desires write access, true for write mode, false for read mode
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
 * @brief Opens file with last given filename, reads bytes to an array, allocates
 * memory only if *bytes is too small
 * 
 * @param self Pointer to femtoFile_t structure
 * @param bytes Address of pointer to character array
 * @param bytesLen Address of array length in bytes
 * @return const wchar_t* Error message, NULL on success
 */
const wchar_t * femtoFile_readBytes(femtoFile_t * restrict self, char ** bytes, uint32_t * bytesLen);
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
 * @return int Negative values represent error code, positive values (0 inclusive)
 * represent number of bytes written to disc
 */
int femtoFile_write(femtoFile_t * restrict self);
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
 * @return true Success
 * @return false Failure
 */
bool femtoFile_addSpecialCh(femtoFile_t * restrict self, uint32_t height, wchar_t ch);

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
 * @return true Success
 * @return false Failure
 */
bool femtoFile_addNewLine(femtoFile_t * restrict self);
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

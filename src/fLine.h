#ifndef FEMTOLINE_H
#define FEMTOLINE_H

#include "fCommon.h"
#include "fSyntax.h"

// Forward-declare from fFile.h
struct fFileHighLight;

#define FEMTO_LNODE_DEFAULT_FREE 10


typedef struct fLine
{
	wchar * line;
	usize lineEndx, curx, freeSpaceLen;
	struct fLine * prevNode, * nextNode;
	usize virtcurx;

	usize lineNumber;

	WORD * syntax;
	union
	{
		u8 val;
		struct
		{
			u8 b1:1;
			u8 b2:1;
			u8 b3:1;
			u8 b4:1;
			u8 b5:1;
			u8 b6:1;
			u8 b7:1;
			u8 b8:1;
		} bits;
	} userValue;

} fLine_t;

typedef enum fSearch
{
	fsrchFIRST,
	fsrchLAST,
	fsrchPREV,
	fsrchNEXT
} fSearch_e;


/**
 * @brief Initialises already pre-allocated memory of fLine_t structure
 * 
 * @param self Pointer to line node
 */
void fLine_init(fLine_t * restrict self);
/**
 * @brief Creates new line in-between current line and next line
 * 
 * @param curnode Pointer to current line node, can be NULL
 * @param nextnode Pointer to next line node, can be NULL
 * @param tabsToSpaces Tabs are converted to spaces
 * @param tabWidth Tab character width in spaces
 * @param autoIndent Automatically insert tabs to match whitespace of the previous line
 * @param noLen Pointer receiving last line number length in characters
 * @return fLine_t* Pointer to newly created line node, NULL on failure
 */
fLine_t * fLine_create(
	fLine_t * restrict curnode,
	fLine_t * restrict nextnode,
	bool tabsToSpaces,
	u8 tabWidth,
	bool autoIndent,
	u8 * noLen
);
/**
 * @brief Creates new line in-between current line and next line
 * 
 * @param curnode Pointer to current line node, can be NULL
 * @param nextnode Pointer to next line node, can be NULL
 * @param lineText Pointer to UTF-16 character array, contents of this will be
 * copied to the newly created line
 * @param maxText Maximum amount of characters to copy (not including null-terminator),
 * can be -1, if string is null-terminated
 * @param noLen Pointer receiving last line number length in characters
 * @return fLine_t* Pointer to newly created line node, NULL on failure
 */
fLine_t * fLine_createText(
	fLine_t * restrict curnode,
	fLine_t * restrict nextnode,
	const wchar * restrict lineText,
	isize maxText,
	u8 * restrict noLen
);

/**
 * @brief Fetches text from given line node, copies it to wchar character array,
 * allocates memory only if *text is too small or tarrsz == NULL
 * 
 * @param self Pointer to line node to fetch text from
 * @param text Address of wchar pointer to character array, wchar pointer
 * itself can be initally NULL
 * @param tarrsz Size of receiving character array, can be NULL
 * @return true Success
 * @return false Failure
 */
bool fLine_getText(const fLine_t * restrict self, wchar ** restrict text, usize * restrict tarrsz);
/**
 * @brief Fetches text from given line node, copies it to wchar character array
 * until the length reaches maxLen (including the null-terminator)
 * 
 * @param self Pointer to line node to fetch text from
 * @param text wchar pointer to character array
 * @param maxLen Maximum length of character array in characters (including the null-terminator)
 */
void fLine_getTextLim(const fLine_t * restrict self, wchar * restrict text, usize maxLen);
/**
 * @brief Reallocates free space on given line node, guarantees
 * femto_LNODE_DEFAULT_FREE characters for space
 * 
 * @param self Pointer to line node
 * @return true Success
 * @return false Failure
 */
bool fLine_realloc(fLine_t * restrict self);

/**
 * @brief Add character to line
 * 
 * @param self Pointer to line node
 * @param ch Character to add
 * @param tabWidth Tab character width (to move cursor)
 * @return true Success
 * @return false Failure
 */
bool fLine_addChar(fLine_t * restrict self, wchar ch, u8 tabWidth);

/**
 * @brief Checks current line contents for matching string
 * 
 * @param node Pointer to line node
 * @param maxdelta Offset from current cursor position, value is clamped
 * @param string Pointer to character array to match with
 * @param maxString Absolute maximum number of characters to check, stops anyway on null-terminator
 * @return true Found a match
 * @return false Didn't find any match
 */
bool fLine_checkAt(const fLine_t * restrict node, isize maxdelta, const wchar * restrict string, usize maxString);
/**
 * @brief Finds the string location on current line iterating forwards
 * 
 * @param node Pointer to line node
 * @param startIdx Starting index, value is clamped
 * @param string Matchable string
 * @param maxString Absolute maximum number of character to check, stops anyway on null-terminator
 * @return usize Index location of string, UINT32_MAX on failure
 */
usize fLine_find(const fLine_t * restrict node, usize startIdx, const wchar * restrict string, usize maxString);
/**
 * @brief Finds the string location on current line iterating backwards from the starting point
 * 
 * @param node Pointer to line node
 * @param startIdx Starting index, value is clamped
 * @param string Matchable string
 * @param maxString Absolute maximum number of character to check, stops anyway on null-terminator
 * @return usize Index location of string, UINT32_MAX on failure
 */
usize fLine_rfind(const fLine_t * restrict node, usize startIdx, const wchar * restrict string, usize maxString);


/**
 * @brief Merges current line node with next line node, adjusts current
 * y-position line node pointer if necessary
 * 
 * @param self Pointer to current line node
 * @param ppcury Address of pointer to current y-position line node
 * @param noLen Pointer receiving last line number length in characters
 * @return true Success
 * @return false Failure
 */
bool fLine_mergeNext(fLine_t * restrict self, fLine_t ** restrict ppcury, u8 * restrict noLen);

/**
 * @brief Moves (internal) cursor on current line node, clamps movement
 * 
 * @param self Pointer to current line node
 * @param delta Amount of characters to move, positive values to move right,
 * negative values to move left
 */
void fLine_moveCursor(fLine_t * restrict self, isize delta);
/**
 * @brief Moves (internal) cursor on current line node, clamps movement
 * 
 * @param self Pointer to current line node
 * @param curx Absolute cursor position to move to
 */
void fLine_moveCursorAbs(fLine_t * restrict self, usize curx);
/**
 * @brief Moves active line node, clamps movement
 * 
 * @param self Address of pointer to current line node
 * @param delta Amount of lines to move, positive values move down, negative values move up
 */
void fLine_moveCursorVert(fLine_t ** restrict self, isize delta);
/**
 * @brief Calculates virtual cursor location
 * 
 * @param self Pointer to current line node
 * @param tabWidth Tab character width in spaces equivalent
 */
void fLine_calcVirtCursor(fLine_t * restrict self, usize tabWidth);
/**
 * @brief Calculates real cursor location from virtual cursor location
 * 
 * @param self Pointer to current line node
 * @param virtcur Virtual cursor position
 * @param tabWidth Tab character width in spaces equivalent
 * @return usize Real cursor location
 */
usize fLine_calcCursor(const fLine_t * restrict self, usize virtcur, u8 tabWidth);

/**
 * @brief Swaps internally data between 2 nodes
 * 
 * @param node1 First node
 * @param node2 Second node
 */
void fLine_swap(fLine_t * restrict node1, fLine_t * restrict node2);

/**
 * @brief Updates line numbers starting from given startnode (inclusive)
 * 
 * @param startnode Starting node for line number updater
 * @param startLno Starting line number
 * @param noLen Pointer receiving last line number length in characters
 */
void fLine_updateLineNumbers(fLine_t * restrict startnode, usize startLno, u8 * restrict noLen);

/**
 * @brief Updates the syntax highlighting of a given node
 * 
 * @param node Pointer to node
 * @param fs Syntax identifier
 * @param colors Syntax token coloring palette
 * @param searchTerm Phrase to be searched, can be NULL
 * @param searchOpts Search options
 * @param hl Highlighting data
 * @param curLineNum Current active line number
 * @param tabWidth Tab width in characters
 * @return true Success
 * @return false Failure, usually when allocating memory
 */
bool fLine_updateSyntax(
	fLine_t * restrict node, fStx_e fs, const WORD * colors,
	const wchar * restrict searchTerm, fSearch_e searchOpts, const struct fFileHighLight * restrict hl,
	usize curLineNum, u8 tabWidth
);

/**
 * @brief Destroys line node, does NOT free any memory
 * 
 * @param self Pointer to line node
 */
void fLine_destroy(fLine_t * restrict self);
/**
 * @brief Destroys line node, frees memory
 * 
 * @param self Pointer to line node
 */
void fLine_free(fLine_t * restrict self);


#endif

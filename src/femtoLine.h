#ifndef FEMTOLINE_H
#define FEMTOLINE_H

#include "common.h"
#include "femtoSyntax.h"

#define FEMTO_LNODE_DEFAULT_FREE 10


typedef struct femtoLineNode
{
	wchar_t * line;
	uint32_t lineEndx, curx, freeSpaceLen;
	struct femtoLineNode * prevNode, * nextNode;
	uint32_t virtcurx;

	uint32_t lineNumber;

	WORD * syntax;
	bool bBlockComment;

} femtoLineNode_t;


/**
 * @brief Initialises already pre-allocated memory of femtoLineNode_t structure
 * 
 * @param self Pointer to line node
 */
void femtoLine_init(femtoLineNode_t * restrict self);
/**
 * @brief Creates new line in-between current line and next line
 * 
 * @param curnode Pointer to current line node, can be NULL
 * @param nextnode Pointer to next line node, can be NULL
 * @param tabsToSpaces Tabs are converted to spaces
 * @param tabWidth Tab character width in spaces
 * @param autoIndent Automatically insert tabs to match whitespace of the previous line
 * @param noLen Pointer receiving last line number length in characters
 * @return femtoLineNode_t* Pointer to newly created line node, NULL on failure
 */
femtoLineNode_t * femtoLine_create(
	femtoLineNode_t * restrict curnode,
	femtoLineNode_t * restrict nextnode,
	bool tabsToSpaces,
	uint8_t tabWidth,
	bool autoIndent,
	uint8_t * noLen
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
 * @return femtoLineNode_t* Pointer to newly created line node, NULL on failure
 */
femtoLineNode_t * femtoLine_createText(
	femtoLineNode_t * restrict curnode,
	femtoLineNode_t * restrict nextnode,
	const wchar_t * restrict lineText,
	int32_t maxText,
	uint8_t * restrict noLen
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
 * @brief Fetches text from given line node, copies it to wchar_t character array
 * until the length reaches maxLen (including the null-terminator)
 * 
 * @param self Pointer to line node to fetch text from
 * @param text wchar_t pointer to character array
 * @param maxLen Maximum length of character array in characters (including the null-terminator)
 */
void femtoLine_getTextLim(const femtoLineNode_t * restrict self, wchar_t * restrict text, uint32_t maxLen);
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
 * @brief Add character to line
 * 
 * @param self Pointer to line node
 * @param ch Character to add
 * @param tabWidth Tab character width (to move cursor)
 * @return true Success
 * @return false Failure
 */
bool femtoLine_addChar(femtoLineNode_t * restrict self, wchar_t ch, uint32_t tabWidth);

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
bool femtoLine_checkAt(const femtoLineNode_t * restrict node, int32_t maxdelta, const wchar_t * restrict string, uint32_t maxString);

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
bool femtoLine_mergeNext(femtoLineNode_t * restrict self, femtoLineNode_t ** restrict ppcury, uint8_t * restrict noLen);

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
 * @brief Calculates virtual cursor location
 * 
 * @param self Pointer to current line node
 * @param tabWidth Tab character width in spaces equivalent
 */
void femtoLine_calcVirtCursor(femtoLineNode_t * restrict self, uint32_t tabWidth);
/**
 * @brief Calculates real cursor location from virtual cursor location
 * 
 * @param self Pointer to current line node
 * @param virtcur Virtual cursor position
 * @param tabWidth Tab character width in spaces equivalent
 * @return uint32_t Real cursor location
 */
uint32_t femtoLine_calcCursor(const femtoLineNode_t * restrict self, uint32_t virtcur, uint32_t tabWidth);

/**
 * @brief Swaps internally data between 2 nodes
 * 
 * @param node1 First node
 * @param node2 Second node
 */
void femtoLine_swap(femtoLineNode_t * restrict node1, femtoLineNode_t * restrict node2);

/**
 * @brief Updates line numbers starting from given startnode (inclusive)
 * 
 * @param startnode Starting node for line number updater
 * @param startLno Starting line number
 * @param noLen Pointer receiving last line number length in characters
 */
void femtoLine_updateLineNumbers(femtoLineNode_t * restrict startnode, uint32_t startLno, uint8_t * restrict noLen);

/**
 * @brief Updates the syntax highlighting of a given node
 * 
 * @param node Pointer to node
 * @param fs Syntax identifier
 * @return true Success
 * @return false Failure, usually when allocating memory
 */
bool femtoLine_updateSyntax(femtoLineNode_t * restrict node, enum femtoSyntax fs);

/**
 * @brief Destroys line node, does NOT free any memory
 * 
 * @param self Pointer to line node
 */
void femtoLine_destroy(femtoLineNode_t * restrict self);
/**
 * @brief Destroys line node, frees memory
 * 
 * @param self Pointer to line node
 */
void femtoLine_free(femtoLineNode_t * restrict self);


#endif

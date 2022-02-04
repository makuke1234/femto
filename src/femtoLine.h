#ifndef FEMTOLINE_H
#define FEMTOLINE_H

#include "common.h"

#define FEMTO_LNODE_DEFAULT_FREE 10

/*
	Example:
	L"This is text\0\0\0\0\0\0"
	              ^ - curx
				  <----------> - freeSpaceLen = 6
*/

typedef struct femtoLineNode
{
	wchar_t * line;
	uint32_t lineEndx, curx, freeSpaceLen;
	struct femtoLineNode * prevNode, * nextNode;
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


#endif

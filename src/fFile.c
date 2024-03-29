#include "fFile.h"
#include "femto.h"
#include "winarr.h"

void fFile_reset(fFile_t * restrict self)
{
	assert(self != NULL);

	(*self) = (fFile_t){
		.fileName = NULL,
		.hFile    = INVALID_HANDLE_VALUE,
		.data     = {
			.firstNode   = NULL,
			.currentNode = NULL,
			.pcury       = NULL,
			.curx        = 0,
			.lastx       = 0,
			.noLen       = 0,
			.hl = {
				.beg       = NULL,
				.begx      = 0,
				.backwards = false
			},
			.bTyped      = false,
			.bUpdateAll  = false
		},
		.eolSeq        = eolNOT,
		.bExists       = false,
		.bCanWrite     = false,
		.bUnsaved      = false,
		.bSyntaxByUser = false,
		.syntax        = fstxNONE
	};
}
fFile_t * fFile_resetDyn(void)
{
	fFile_t * file = malloc(sizeof(fFile_t));
	if (file == NULL)
	{
		return NULL;
	}

	fFile_reset(file);
	return file;
}
bool fFile_open(fFile_t * restrict self, const wchar * restrict fileName, bool writemode)
{
	assert(self != NULL);

	fileName = (fileName == NULL) ? self->fileName : fileName;

	// Get syntax type from file suffix
	if (!self->bSyntaxByUser && (fileName != NULL))
	{
		self->syntax = fStx_detect(fileName);
	}

	if ((fileName != NULL) && ((self->bExists = femto_testFile(fileName)) || writemode))
	{
		// try to open file
		self->hFile = femto_openFile(fileName, writemode);
		if (self->hFile == INVALID_HANDLE_VALUE)
		{
			self->bCanWrite = false;
			return false;
		}
	}

	self->bCanWrite = writemode;
	self->fileName  = (fileName != NULL) ? wcsredup(self->fileName, fileName) : NULL;

	// Only fail if file name was given but the saving of the filename was unsuccessful
	return !((fileName != NULL) && (self->fileName == NULL));
}
void fFile_close(fFile_t * restrict self)
{
	assert(self != NULL);

	if (self->hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(self->hFile);
		self->hFile = INVALID_HANDLE_VALUE;
	}
}
void fFile_clearLines(fFile_t * restrict self)
{
	assert(self != NULL);

	fLine_t * restrict node = self->data.firstNode;
	self->data.firstNode   = NULL;
	self->data.currentNode = NULL;
	self->data.pcury       = NULL;
	while (node != NULL)
	{
		fLine_t * restrict next = node->nextNode;
		fLine_free(node);
		node = next;
	}
}
const wchar * fFile_readBytes(fFile_t * restrict self, char ** restrict bytes, usize * restrict bytesLen)
{
	assert(self      != NULL);
	assert(bytes     != NULL);
	assert((bytesLen != NULL) && "Pointer to length variable is mandatory!");

	if (fFile_open(self, NULL, false) == false)
	{
		return L"File opening error!";
	}

	if (self->bExists)
	{
		const wchar * result = femto_readBytes(self->hFile, bytes, bytesLen);
		fFile_close(self);
		return result;
	}
	else
	{
		*bytes = malloc(1 * sizeof(char));
		if (*bytes == NULL)
		{
			return L"Memory error!";
		}

		(*bytes)[0] = '\0';
		*bytesLen = 1;
		return NULL;
	}
}
const wchar * fFile_read(fFile_t * restrict self)
{
	assert(self != NULL);

	char * bytes = NULL;
	usize size;
	const wchar * restrict res = fFile_readBytes(self, &bytes, &size);
	if (res != NULL)
	{
		return res;
	}

	// Convert to UTF-16
	wchar * utf16 = NULL;
	const u32 chars = femto_toutf16(bytes, (int)size, &utf16, NULL);
	free(bytes);

	if (utf16 == NULL)
	{
		return L"Unicode conversion error!";
	}
	fLog_write("Converted %u bytes of character to %u UTF-16 characters.", size, chars);

	// Save lines to structure
	wchar ** lines = NULL;
	const usize numLines = femto_strnToLines(utf16, chars, &lines, &self->eolSeq);
	if (lines == NULL)
	{
		free(utf16);
		return L"Line reading error!";
	}
	fLog_write("Total of %zu lines", numLines);

	fFile_clearLines(self);
	if (numLines == 0)
	{
		self->data.firstNode = fLine_create(NULL, NULL, false, 0, false, &self->data.noLen);
		if (self->data.firstNode == NULL)
		{
			free(lines);
			free(utf16);
			return L"Line creation error!";
		}
	}
	else
	{
		self->data.firstNode = fLine_createText(NULL, NULL, lines[0], -1, &self->data.noLen);
		if (self->data.firstNode == NULL)
		{
			free(lines);
			free(utf16);
			return L"Line creation error!";
		}
	}
	self->data.currentNode = self->data.firstNode;
	for (usize i = 1; i < numLines; ++i)
	{
		fLine_t * node = fLine_createText(self->data.currentNode, NULL, lines[i], -1, &self->data.noLen);
		if (node == NULL)
		{
			free(lines);
			free(utf16);
			return L"Line creation error!";
		}
		self->data.currentNode = node;
	}
	free(lines);
	free(utf16);

	return NULL;
}
ffcr_e fFile_checkUnsaved(fFile_t * restrict self, char ** editorContents, usize * editorContLen)
{
	assert(self != NULL);

	// Generate lines
	wchar * lines = NULL, * line = NULL;
	usize linesCap = 0, linesLen = 0, lineCap = 0;

	const fLine_t * restrict node = self->data.firstNode;

	const eolSeq_e eolSeq = self->eolSeq;
	bool isCRLF = (self->eolSeq == eolCRLF);

	while (node != NULL)
	{
		if (fLine_getText(node, &line, &lineCap) == false)
		{
			fLog_write("Failed to fetch line!");
			if (line != NULL)
			{
				free(line);
			}
			if (lines != NULL)
			{
				free(lines);
			}
			return ffcrMEM_ERROR;
		}

		const usize lineLen = wcsnlen(line, lineCap);

		const usize addnewline = (node->nextNode != NULL) ? (usize)(1 + (usize)isCRLF) : 0;
		const usize newLinesLen = linesLen + lineLen + addnewline;

		// Add line to lines, concatenate \n character, if necessary

		if (newLinesLen >= linesCap)
		{
			// Resize lines array
			const usize newCap = (newLinesLen + 1) * 2;

			vptr mem = realloc(lines, sizeof(wchar) * newCap);
			if (mem == NULL)
			{
				if (line != NULL)
				{
					free(line);
				}
				if (lines != NULL)
				{
					free(lines);
				}
				return ffcrMEM_ERROR;
			}

			lines    = mem;
			linesCap = newCap;
		}

		// Copy line
		memcpy(lines + linesLen, line, sizeof(wchar) * lineLen);
		linesLen = newLinesLen;

		if (addnewline)
		{
			switch (eolSeq)
			{
			case eolCR:
				lines[linesLen - 1] = L'\r';
				break;
			case eolLF:
				lines[linesLen - 1] = L'\n';
				break;
			case eolCRLF:
				lines[linesLen - 2] = L'\r';
				lines[linesLen - 1] = L'\n';
				break;
			case eolNOT:
				assert(!"EOL sequence not selected!");
				break;
			}
		}
		lines[linesLen] = L'\0';

		node = node->nextNode;
	}
	free(line);

	// Try to convert lines string to UTF-8
	char * utf8 = NULL;
	usize utf8sz = 0;
	femto_toutf8(lines, (int)linesLen + 1, &utf8, &utf8sz);

	// Free UTF-16 lines string
	free(lines);

	// Error-check conversion
	if (utf8 == NULL)
	{
		return ffcrMEM_ERROR;
	}

	// Check if anything has changed, for that load original file again
	char * compFile = NULL;
	usize compSize;
	if (fFile_readBytes(self, &compFile, &compSize) == NULL)
	{
		// Reading was successful
		const bool areEqual = strncmp(utf8, compFile, min_usize(utf8sz, compSize)) == 0;
		free(compFile);

		if (areEqual)
		{
			// Free all resources before returning
			free(utf8);
			self->bUnsaved = false;
			return ffcrNOTHING_NEW;
		}
	}

	if ((editorContents != NULL) && (editorContLen != NULL))
	{
		*editorContents = utf8;
		*editorContLen  = utf8sz;
	}
	else
	{
		free(utf8);
	}

	self->bUnsaved = true;
	return ffcrNEEDS_SAVING;
}
isize fFile_write(fFile_t * restrict self)
{
	assert(self != NULL);

	char * utf8 = NULL;
	usize utf8sz = 0;
	ffcr_e checkres = fFile_checkUnsaved(self, &utf8, &utf8sz);
	checkres = ((checkres == ffcrNOTHING_NEW) && !self->bExists) ? ffcrNEEDS_SAVING : checkres;
	switch (checkres)
	{
	case ffcrMEM_ERROR:
		return ffwrMEM_ERROR;
	case ffcrNOTHING_NEW:
		return ffwrNOTHING_NEW;
	case ffcrNEEDS_SAVING:
		break;
	}

	// Try to open file for writing
	if (fFile_open(self, NULL, true) == false)
	{
		return ffwrOPEN_ERROR;
	}

	if (self->bCanWrite == false)
	{
		fFile_close(self);
		return ffwrWRITE_ERROR;
	}

	fLog_write("Opened file for writing");

	// Try to write UTF-8 lines string to file
	DWORD dwWritten = 0;

	// Write everything except the null terminator
	BOOL res = (utf8 != NULL) ? WriteFile(
		self->hFile,
		utf8,
		(DWORD)(utf8sz - (usize)1),
		&dwWritten,
		NULL
	) : TRUE;
	// Close file
	fFile_close(self);
	// Free utf8 string
	free(utf8);

	// Do error checking
	if (!res)
	{
		fLog_write("Error writing to file");
		return ffwrWRITE_ERROR;
	}
	else
	{
		self->bUnsaved = false;
		return (isize)dwWritten;
	}
}


bool fFile_addNormalCh(fFile_t * restrict self, wchar ch, u8 tabWidth)
{
	assert(self     != NULL);
	assert(tabWidth > 0);

	fLine_t * restrict node = self->data.currentNode;
	assert(node != NULL);
	self->data.bTyped = true;

	fLog_write("Add character %C", ch);

	if (!fLine_addChar(node, ch, tabWidth))
	{
		return false;
	}
	self->data.lastx = node->virtcurx;
	return true;
}


bool fFile_startHighlighting(fFile_t * restrict self, wchar ch, bool shift)
{
	assert(self != NULL);

	struct fFileHighLight * restrict hl = &self->data.hl;
	assert(hl != NULL);

	if ((ch == VK_DELETE) || (ch == FEMTO_SEL_DELETE) || (ch == VK_BACK))
	{
		return hl->beg != NULL;
	}
	else if ( (ch == VK_LEFT) || (ch == VK_RIGHT) || (ch == VK_UP) || (ch == VK_DOWN) )
	{

		const fLine_t * restrict node = self->data.currentNode;
		assert(node != NULL);

		// Cancel highlighting
		if (!shift)
		{
			hl->beg = NULL;
		}
		else
		{
			// Save cursor position for highlighting
			if (hl->beg == NULL)
			{
				hl->beg  = node;
				hl->begx = node->curx & USIZE_BIT_1_MASK;
			}

			hl->backwards = ((hl->beg == node) && ((ch == VK_UP) || ((hl->begx > node->curx) || ((hl->begx == node->curx) && (ch == VK_LEFT)))) ) ||
				(hl->beg->lineNumber > node->lineNumber);
		}
	}

	return hl->beg != NULL;
}


bool fFile_addSpecialCh(
	fFile_t * restrict self, u32 height,
	wchar ch, bool shift,
	const fSettings_t * pset
)
{
	assert(self   != NULL);
	assert(height > 0);
	assert(pset   != NULL);

	self->data.bTyped = true;
	fLine_t * restrict lastcurnode = self->data.currentNode;
	assert(lastcurnode != NULL);

	const fLine_t * restrict prevbeg = self->data.hl.beg;
	fFile_startHighlighting(self, ch, shift);

	switch (ch)
	{
	case VK_TAB:
	{
		const wchar tch = L"\t "[pset->bTabsToSpaces];
		if (!fFile_addNormalCh(self, tch, pset->tabWidth))
		{
			return false;
		}
		if (pset->bTabsToSpaces)
		{
			for (usize i = 0, max = pset->tabWidth - (lastcurnode->virtcurx % pset->tabWidth); i < max; ++i)
			{
				if (fFile_addNormalCh(self, tch, pset->tabWidth) == false)
				{
					return false;
				}
			}
		}
		fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	}
	case VK_OEM_BACKTAB:
		// Check if there's 4 spaces before the caret
		if (fLine_checkAt(lastcurnode, -1, L" ", 1))
		{
			fLog_write("Back-tabbing SPACE before caret");
			fFile_deleteBackward(self);
			--lastcurnode->virtcurx;
			const usize max = lastcurnode->virtcurx % pset->tabWidth;
			if (fLine_checkAt(lastcurnode, -(isize)max, pset->tabSpaceStr1, max))
			{
				for (usize i = 0; i < max; ++i)
				{
					fFile_deleteBackward(self);
				}
			}
		}
		else if (fLine_checkAt(lastcurnode, -1, L"\t", 1))
		{
			fLog_write("Back-tabbing TAB before caret");
			fFile_deleteBackward(self);
		}
		// If there isn't, check if there's up to 4 spaces after the caret
		else if (fLine_checkAt(lastcurnode, 0, L" ", 1))
		{
			fLog_write("Back-tabbing SPACE after caret");
			for (usize i = 0; i < pset->tabWidth; ++i)
			{
				if (!fLine_checkAt(lastcurnode, 0, L" ", 1))
				{
					break;
				}
				fFile_deleteForward(self);
			}
		}
		else if (fLine_checkAt(lastcurnode, 0, L"\t", 1))
		{
			fLog_write("Back-tabbing TAB after caret");
			fFile_deleteForward(self);
		}

		fLog_write("Calculating new virtual cursor");
		fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_RETURN:	// Enter key
		fFile_addNewLine(self, pset->bTabsToSpaces, pset->tabWidth, pset->bAutoIndent);
		fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_BACK:	// Backspace
		if (self->data.hl.beg != NULL)
		{
			return fFile_addSpecialCh(self, height, FEMTO_SEL_DELETE, shift, pset);
		}
		else
		{
			fFile_deleteBackward(self);
			fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
			self->data.lastx = self->data.currentNode->virtcurx;
		}
		break;
	case VK_DELETE:	// Delete
		if (self->data.hl.beg != NULL)
		{
			return fFile_addSpecialCh(self, height, FEMTO_SEL_DELETE, shift, pset);
		}
		else
		{
			fFile_deleteForward(self);
		}
		break;
	case FEMTO_SHIFT_DEL:	// Shift+Delete
		fFile_deleteLine(self);
		break;
	case FEMTO_SEL_DELETE:	// Delete selection
		fFile_deleteSelection(self);
		fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case FEMTO_COPY:	 // Copy selection to clipboard
		if (!fFile_cbCopy(self))
		{
			return false;
		}
		break;
	case FEMTO_PASTE:	// Paste from clipboard
		if (!fFile_cbPaste(self, height, pset))
		{
			return false;
		}
		break;
	case FEMTO_MOVELINE_UP:
		// Swap current line with previous if possible
		if (lastcurnode->prevNode != NULL)
		{
			fLine_swap(lastcurnode, self->data.currentNode->prevNode);
			self->data.currentNode = self->data.currentNode->prevNode;
			fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
			self->data.lastx       = self->data.currentNode->virtcurx;
			self->data.bUpdateAll  = true;
		}
		break;
	case FEMTO_MOVELINE_DOWN:
		// Swap current line with next if possible
		if (lastcurnode->nextNode != NULL)
		{
			fLine_swap(lastcurnode, self->data.currentNode->nextNode);
			self->data.currentNode = self->data.currentNode->nextNode;
			fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
			self->data.lastx       = self->data.currentNode->virtcurx;
			self->data.bUpdateAll  = true;
		}
		break;
	case VK_LEFT:	// Left arrow
		if (lastcurnode->curx > 0)
		{
			fLine_moveCursor(lastcurnode, -1);
		}
		else if (lastcurnode->prevNode != NULL)
		{
			self->data.currentNode = lastcurnode->prevNode;
			fLine_moveCursor(self->data.currentNode, (isize)self->data.currentNode->lineEndx);
		}
		fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_RIGHT:	// Right arrow
		if ((lastcurnode->curx + lastcurnode->freeSpaceLen) < lastcurnode->lineEndx)
		{
			fLine_moveCursor(lastcurnode, 1);
		}
		else if (lastcurnode->nextNode != NULL)
		{
			self->data.currentNode = self->data.currentNode->nextNode;
			fLine_moveCursor(self->data.currentNode, -(isize)self->data.currentNode->lineEndx);
		}
		fLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_UP:		// Up arrow
		fLine_moveCursorVert(&self->data.currentNode, -1);
		fLine_moveCursorAbs(self->data.currentNode, fLine_calcCursor(self->data.currentNode, self->data.lastx, pset->tabWidth));
		break;
	case VK_DOWN:	// Down arrow
		fLine_moveCursorVert(&self->data.currentNode, 1);
		fLine_moveCursorAbs(self->data.currentNode, fLine_calcCursor(self->data.currentNode, self->data.lastx, pset->tabWidth));
		break;
	case VK_PRIOR:	// Page up
		fLine_moveCursorVert(&self->data.currentNode, -(isize)height);
		fLine_moveCursorAbs(self->data.currentNode, fLine_calcCursor(self->data.currentNode, self->data.lastx, pset->tabWidth));
		break;
	case VK_NEXT:	// Page down
		fLine_moveCursorVert(&self->data.currentNode, (isize)height);
		fLine_moveCursorAbs(self->data.currentNode, fLine_calcCursor(self->data.currentNode, self->data.lastx, pset->tabWidth));
		break;
	case VK_END:
		fLine_moveCursor(lastcurnode, (isize)lastcurnode->lineEndx);
		fLine_calcVirtCursor(lastcurnode, pset->tabWidth);
		self->data.lastx = lastcurnode->virtcurx;
		break;
	case VK_HOME:
		fLine_moveCursor(lastcurnode, -(isize)lastcurnode->lineEndx);
		fLine_calcVirtCursor(lastcurnode, pset->tabWidth);
		self->data.lastx = lastcurnode->virtcurx;
		break;
	default:
		return false;
	}

	self->data.bUpdateAll |= ((self->data.currentNode != lastcurnode) & pset->bRelLineNums) ||
		((self->data.hl.beg != NULL) && (self->data.hl.beg != self->data.currentNode)) ||
		(self->data.hl.beg != prevbeg);

	return true;
}

bool fFile_deleteForward(fFile_t * restrict self)
{
	assert(self != NULL);

	fLine_t * restrict node = self->data.currentNode;
	assert(node != NULL);

	if ((node->curx + node->freeSpaceLen) < node->lineEndx)
	{
		++node->freeSpaceLen;
		return true;
	}
	else if (node->nextNode != NULL)
	{
		self->data.bUpdateAll = true;
		return fLine_mergeNext(node, &self->data.pcury, &self->data.noLen);
	}
	else
	{
		return false;
	}
}
bool fFile_deleteBackward(fFile_t * restrict self)
{
	assert(self != NULL);

	fLine_t * restrict node = self->data.currentNode;
	assert(node != NULL);

	if (node->curx > 0)
	{
		--node->curx;
		++node->freeSpaceLen;
		return true;
	}
	else if (node->prevNode != NULL)
	{
		// Add current node data to previous node data
		self->data.currentNode = node->prevNode;
		self->data.bUpdateAll  = true;
		return fLine_mergeNext(self->data.currentNode, &self->data.pcury, &self->data.noLen);
	}
	else
	{
		return false;
	}
}
bool fFile_deleteLine(fFile_t * restrict self)
{
	assert(self != NULL);

	fLine_t * restrict node = self->data.currentNode;
	assert(node != NULL);

	if (node->nextNode != NULL)
	{
		if (self->data.pcury == node)
		{
			self->data.pcury = self->data.pcury->nextNode;
		}
		self->data.currentNode = node->nextNode;
		self->data.currentNode->prevNode = node->prevNode;
		if (node->prevNode != NULL)
		{
			node->prevNode->nextNode = self->data.currentNode;
		}
		else if (node == self->data.firstNode)
		{
			self->data.firstNode = self->data.currentNode;
		}
		--self->data.currentNode->lineNumber;

		// Destroy current line
		fLine_free(node);
		self->data.bUpdateAll = true;
	}
	else if (node->prevNode != NULL)
	{
		if (self->data.pcury == node)
		{
			self->data.pcury = self->data.pcury->prevNode;
		}
		self->data.currentNode = node->prevNode;
		self->data.currentNode->nextNode = NULL;

		fLine_free(node);
		self->data.bUpdateAll = true;
	}
	if (self->data.bUpdateAll)
	{
		fLine_updateLineNumbers(self->data.currentNode, self->data.currentNode->lineNumber, &self->data.noLen);
	}
	return true;
}
bool fFile_deleteSelection(fFile_t * restrict self)
{
	assert(self != NULL);

	struct fFileHighLight * restrict hl = &self->data.hl;
	assert(hl != NULL);

	const fLine_t * restrict begNode = hl->beg;
	const usize begNodex = hl->begx + ((hl->begx > hl->beg->curx) ? hl->beg->freeSpaceLen : 0);

	assert(begNode != NULL);

	const fLine_t * restrict node = self->data.currentNode;
	assert(node != NULL);

	if (hl->backwards)
	{
		fLog_write("Backwards selection delete");
		// Use delete
		// Calculate how many times to hit delete
		usize rep = 0;

		usize idx = node->curx;
		usize curx = idx;

		while ((node != begNode) || (idx < begNodex))
		{

			if (idx == node->lineEndx)
			{
				node = node->nextNode;
				assert(node != NULL);
				idx = 0;
				curx = node->curx;
			}
			else
			{
				++rep;
				if ((idx == curx) && (node->freeSpaceLen > 0))
				{
					idx += node->freeSpaceLen;
				}
				else
				{
					++idx;
				}
			}

		}
		fLog_write("%zu deletions", rep);
		for (usize i = 0; i < rep; ++i)
		{
			if (!fFile_deleteForward(self))
			{
				return false;
			}
		}
	}
	else
	{
		// Use delete
		// Calculate how many times to hit delete
		usize rep = 0;
		usize curx = node->curx + node->freeSpaceLen;
		usize idx = node->curx;

		while ((node != begNode) || (idx > begNodex))
		{
			if (idx == 0)
			{
				node = node->prevNode;
				assert(node != NULL);
				idx = node->lineEndx;
				curx = node->curx + node->freeSpaceLen;
			}
			else
			{
				++rep;
				if ((idx == curx) && (node->freeSpaceLen > 0))
				{
					idx -= node->freeSpaceLen;
				}
				else
				{
					--idx;
				}
			}
		}
		fLog_write("%zu deletions", rep);
		for (usize i = 0; i < rep; ++i)
		{
			if (!fFile_deleteBackward(self))
			{
				return false;
			}
		}
	}

	hl->beg = NULL;
	return true;
}

bool fFile_cbCopy(fFile_t * restrict self)
{
	assert(self != NULL);

	if (!OpenClipboard(GetConsoleWindow()))
	{
		return false;
	}
	EmptyClipboard();

	struct fFileHighLight * restrict hl = &self->data.hl;
	assert(hl != NULL);

	const fLine_t * restrict beg = hl->beg;

	if (beg == NULL)
	{
		fLog_write("Nothing selected, nothing to copy");
		CloseClipboard();
		return false;
	}

	// Copy "stuff" onto clipboard

	const fLine_t * restrict curnode = self->data.currentNode;
	assert(curnode != NULL);

	// Find first & last node
	const fLine_t * restrict begnode = NULL, * restrict endnode = NULL;
	if (beg->lineNumber < curnode->lineNumber)
	{
		begnode = hl->beg;
		endnode = curnode;
	}
	else
	{
		begnode = curnode;
		endnode = hl->beg;
	}

	assert(begnode != NULL);
	assert(endnode != NULL);

	// Initialize clipboard buffer
	warr_t clipBuf = { 0 };
	warr_init(&clipBuf, sizeof(wchar));

	while (1)
	{
		// Calculate begining cursor & ending cursor
		usize begCur, endCur;

		if (curnode == beg)
		{
			begCur = hl->backwards ? begnode->curx : hl->begx;
			endCur = hl->backwards ? hl->begx      : begnode->curx;
		}
		else if (begnode == beg)
		{
			begCur = hl->backwards ? 0 : hl->begx;
			endCur = hl->backwards ? hl->begx : begnode->lineEndx - begnode->freeSpaceLen;
		}
		else if (begnode == curnode)
		{
			begCur = hl->backwards ? begnode->curx                             : 0;
			endCur = hl->backwards ? begnode->lineEndx - begnode->freeSpaceLen : begnode->curx;
		}
		else
		{
			begCur = 0;
			endCur = begnode->lineEndx - begnode->freeSpaceLen;
		}

		begCur += ((begCur >= begnode->curx) && (begnode->freeSpaceLen > 0)) ? begnode->freeSpaceLen : 0;
		endCur += ((endCur >= begnode->curx) && (begnode->freeSpaceLen > 0)) ? begnode->freeSpaceLen : 0;

		while (begCur < endCur)
		{
			if ((begCur == begnode->curx) && (begnode->freeSpaceLen > 0))
			{
				begCur += begnode->freeSpaceLen;
				continue;
			}

			// Add character to clipboard
			if (!warr_pushBack(&clipBuf, &begnode->line[begCur]))
			{
				fLog_write("Error pushing character %C!", begnode->line[begCur]);
				warr_destroy(&clipBuf);
				CloseClipboard();
				return false;
			}

			++begCur;
		}


		if (begnode != endnode)
		{
			// Add newline to clipboard
			if (!warr_pushBack(&clipBuf, &(wchar){ L'\n' }))
			{
				fLog_write("Error pushing newline!");
				warr_destroy(&clipBuf);
				CloseClipboard();
				return false;
			}
		}
		else
		{
			break;
		}
		begnode = begnode->nextNode;
		assert(begnode != NULL);
	};

	// Add null-terminator to clipboard buffer
	if (!warr_pushBack(&clipBuf, &(wchar){ L'\0' }))
	{
		fLog_write("Error pushing null-terminator!");
		warr_destroy(&clipBuf);
		CloseClipboard();
		return false;
	}

	// Save some memory here
	warr_shrinkToFit(&clipBuf);
	fLog_write(
		"Copying %zu characters to clipboard:\n%S\n",
		warr_size(&clipBuf) - 1,
		warr_data(&clipBuf)
	);

	HGLOBAL clipBufMem = warr_unlock(&clipBuf);
	assert(clipBufMem != NULL);

	SetClipboardData(CF_UNICODETEXT, clipBufMem);
	// Finalize operation
	CloseClipboard();

	fLog_write("Copied to clipboard");

	return true;
}
bool fFile_cbPaste(fFile_t * restrict self, u32 height, const fSettings_t * restrict pset)
{
	assert(self != NULL);

	if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
	{
		return true;
	}
	else if (!OpenClipboard(GetConsoleWindow()))
	{
		return false;
	}

	HGLOBAL data = GetClipboardData(CF_UNICODETEXT);
	if (data != NULL)
	{
		const wchar * restrict clipstr = GlobalLock(data);
		if (clipstr != NULL)
		{
			fSettings_t set = *pset;
			set.bAutoIndent = false;
			// Copy text

			for (const wchar * restrict str = clipstr; (*str) != L'\0'; ++str)
			{
				// Add character to current location
				const wchar ch = *str;
				bool ret = true;

				if (ch == L'\r')
				{
					continue;
				}
				else if (ch == L'\n')
				{
					ret = fFile_addSpecialCh(self, height, VK_RETURN, false, &set);
				}
				else
				{
					ret = fFile_addNormalCh(self, ch, set.tabWidth);
				}

				if (!ret)
				{
					fLog_write("Failed pasting on character '%C'", ch);

					GlobalUnlock(data);
					CloseClipboard();
					return false;
				}
			}

			GlobalUnlock(data);
		}
	}
	CloseClipboard();

	return true;
}

bool fFile_addNewLine(fFile_t * restrict self, bool tabsToSpaces, u8 tabWidth, bool autoIndent)
{
	assert(self     != NULL);
	assert(tabWidth > 0);

	fLine_t * node = fLine_create(
		self->data.currentNode,
		self->data.currentNode->nextNode,
		tabsToSpaces,
		tabWidth,
		autoIndent,
		&self->data.noLen
	);
	if (node == NULL)
	{
		return false;
	}

	self->data.currentNode->nextNode = node;
	self->data.currentNode = node;
	self->data.bUpdateAll  = true;
	return true;
}

void fFile_updateCury(fFile_t * restrict self, u32 height)
{
	assert(self   != NULL);
	assert(height > 0);

	if (self->data.currentNode == NULL)
	{
		return;
	}

	if (self->data.pcury == NULL)
	{
		fLine_t * restrict node = self->data.currentNode;
		for (u32 i = 0; i < height && (node->prevNode != NULL); ++i)
		{
			node = node->prevNode;
		}
		self->data.pcury = node;
	}
	else
	{
		const fLine_t * restrict node = self->data.currentNode;
		for (u32 i = 0; i < height && (node != NULL); ++i)
		{
			if (node == self->data.pcury)
			{
				return;
			}
			node = node->prevNode;
		}

		node = self->data.currentNode->nextNode;
		for (; node != NULL;)
		{
			if (node == self->data.pcury)
			{
				self->data.pcury = self->data.currentNode;
				return;
			}
			node = node->nextNode;
		}

		self->data.pcury = NULL;
		fFile_updateCury(self, height);
	}
}
void fFile_scrollVert(fFile_t * restrict self, u32 height, isize deltaLines)
{
	assert(self   != NULL);
	assert(height > 0);

	if (self->data.pcury == NULL)
	{
		fFile_updateCury(self, height);
	}

	if ((deltaLines != 0) && (self->data.pcury != NULL))
	{
		fLine_moveCursorVert(&self->data.pcury, deltaLines);
		self->data.bUpdateAll = true;
	}
}
void fFile_scrollHor(fFile_t * restrict self, u32 width, isize deltaCh)
{
	assert(self  != NULL);
	assert(width > 0);

	if ((deltaCh < 0) && ((usize)-deltaCh <= self->data.curx))
	{
		self->data.curx -= (usize)-deltaCh;
	}
	else if (deltaCh > 0)
	{
		const usize curx = self->data.curx + (usize)deltaCh, total = self->data.currentNode->lineEndx - self->data.currentNode->freeSpaceLen;
		--width;
		if ((total >= width) && (curx <= (total - width)))
		{
			self->data.curx = curx;
		}
	}
	else
	{
		return;
	}
	self->data.bUpdateAll = true;
}


void fFile_destroy(fFile_t * restrict self)
{
	assert(self != NULL);

	fFile_close(self);
	fFile_clearLines(self);

	if (self->fileName != NULL)
	{
		free(self->fileName);
		self->fileName = NULL;
	}
}
void fFile_free(fFile_t * restrict self)
{
	assert(self != NULL);

	fFile_destroy(self);
	free(self);
}

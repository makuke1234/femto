#include "femtoFile.h"
#include "femto.h"

void femtoFile_reset(femtoFile_t * restrict self)
{
	assert(self != NULL);
	(*self) = (femtoFile_t){
		.fileName = NULL,
		.hFile    = INVALID_HANDLE_VALUE,
		.data     = {
			.firstNode   = NULL,
			.currentNode = NULL,
			.pcury       = NULL,
			.curx        = 0,
			.lastx       = 0,
			.noLen       = 0,
			.bTyped      = false,
			.bUpdateAll  = false
		},
		.eolSeq        = eolNOT,
		.bCanWrite     = false,
		.bUnsaved      = false,
		.bSyntaxByUser = false,
		.syntax        = fstxNONE
	};
}
femtoFile_t * femtoFile_resetDyn(void)
{
	femtoFile_t * file = malloc(sizeof(femtoFile_t));
	if (file == NULL)
	{
		return NULL;
	}

	femtoFile_reset(file);
	return file;
}
HANDLE femtoFile_sopen(const wchar_t * restrict fileName, bool writemode)
{
	assert(fileName != NULL);
	HANDLE hfile = CreateFileW(
		fileName,
		writemode ? GENERIC_WRITE : GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		writemode ? CREATE_ALWAYS : OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return hfile;
}
bool femtoFile_open(femtoFile_t * restrict self, const wchar_t * restrict fileName, bool writemode)
{
	assert(self != NULL);
	fileName = (fileName == NULL) ? self->fileName : fileName;

	// Get syntax type from file suffix
	if (!self->bSyntaxByUser && (fileName != NULL))
	{
		self->syntax = fSyntaxDetect(fileName);
	}

	// try to open file
	self->hFile = femtoFile_sopen(fileName, writemode);
	if (self->hFile == INVALID_HANDLE_VALUE)
	{
		self->bCanWrite = false;
		return false;
	}
	else
	{
		self->bCanWrite = writemode;
		self->fileName = wcsredup(self->fileName, fileName);
		if (self->fileName == NULL)
		{
			return false;
		}
		return true;
	}
}
void femtoFile_close(femtoFile_t * restrict self)
{
	assert(self != NULL);
	if (self->hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(self->hFile);
		self->hFile = INVALID_HANDLE_VALUE;
	}
}
void femtoFile_clearLines(femtoFile_t * restrict self)
{
	assert(self != NULL);
	femtoLineNode_t * node = self->data.firstNode;
	self->data.firstNode   = NULL;
	self->data.currentNode = NULL;
	self->data.pcury       = NULL;
	while (node != NULL)
	{
		femtoLineNode_t * next = node->nextNode;
		femtoLine_free(node);
		node = next;
	}
}
const wchar_t * femtoFile_sreadBytes(HANDLE hfile, char ** restrict bytes, uint32_t * restrict bytesLen)
{
	assert(bytes    != NULL);
	assert(bytesLen != NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		return L"File opening error!";
	}
	DWORD fileSize = GetFileSize(hfile, NULL);
	if ((fileSize >= *bytesLen) || (*bytes == NULL))
	{
		void * mem = realloc(*bytes, fileSize + 1);
		if (mem == NULL)
		{
			return L"Memory error!";
		}
		*bytes    = mem;
		*bytesLen = fileSize + 1;
	}

	BOOL readFileRes = ReadFile(
		hfile,
		*bytes,
		fileSize,
		NULL,
		NULL
	);
	if (!readFileRes)
	{
		return L"File read error!";
	}
	// Add null terminator
	(*bytes)[fileSize] = '\0';

	return NULL;
}
const wchar_t * femtoFile_readBytes(femtoFile_t * restrict self, char ** restrict bytes, uint32_t * restrict bytesLen)
{
	assert(self != NULL);
	assert(bytes != NULL);
	assert(bytesLen != NULL && "Pointer to length variable is mandatory!");
	if (femtoFile_open(self, NULL, false) == false)
	{
		return L"File opening error!";
	}
	const wchar_t * result = femtoFile_sreadBytes(self->hFile, bytes, bytesLen);
	femtoFile_close(self);
	
	return result;
}
const wchar_t * femtoFile_read(femtoFile_t * restrict self)
{
	assert(self != NULL);
	char * bytes = NULL;
	uint32_t size;
	const wchar_t * res;
	if ((res = femtoFile_readBytes(self, &bytes, &size)) != NULL)
	{
		return res;
	}

	// Convert to UTF-16
	wchar_t * utf16 = NULL;
	uint32_t chars = femto_convToUnicode(bytes, (int)size, &utf16, NULL);
	free(bytes);

	if (utf16 == NULL)
	{
		return L"Unicode conversion error!";
	}
	writeProfiler("Converted %u bytes of character to %u UTF-16 characters.", size, chars);
	writeProfiler("File UTF-16 contents \"%S\"", utf16);

	// Save lines to structure
	wchar_t ** lines = NULL;
	uint32_t numLines = femto_strnToLines(utf16, chars, &lines, &self->eolSeq);
	if (lines == NULL)
	{
		free(utf16);
		return L"Line reading error!";
	}
	writeProfiler("Total of %u lines", numLines);

	femtoFile_clearLines(self);
	if (numLines == 0)
	{
		self->data.firstNode = femtoLine_create(NULL, NULL, false, 0, false, &self->data.noLen);
		if (self->data.firstNode == NULL)
		{
			free(lines);
			free(utf16);
			return L"Line creation error!";
		}
	}
	else
	{
		self->data.firstNode = femtoLine_createText(NULL, NULL, lines[0], -1, &self->data.noLen);
		if (self->data.firstNode == NULL)
		{
			free(lines);
			free(utf16);
			return L"Line creation error!";
		}
	}
	self->data.currentNode = self->data.firstNode;
	for (uint32_t i = 1; i < numLines; ++i)
	{
		femtoLineNode_t * node = femtoLine_createText(self->data.currentNode, NULL, lines[i], -1, &self->data.noLen);
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
int32_t femtoFile_checkUnsaved(femtoFile_t * restrict self, char ** editorContents, uint32_t * editorContLen)
{
	assert(self != NULL);
	// Generate lines
	wchar_t * lines = NULL, * line = NULL;
	uint32_t linesCap = 0, linesLen = 0, lineCap = 0;

	femtoLineNode_t * node = self->data.firstNode;

	const uint8_t eolSeq = self->eolSeq;
	bool isCRLF = (self->eolSeq == eolCRLF);

	while (node != NULL)
	{
		if (femtoLine_getText(node, &line, &lineCap) == false)
		{
			writeProfiler("Failed to fetch line!");
			if (line != NULL)
			{
				free(line);
			}
			if (lines != NULL)
			{
				free(lines);
			}
			return checkRes_memError;
		}

		uint32_t lineLen = (uint32_t)wcsnlen(line, lineCap);

		uint32_t addnewline = (node->nextNode != NULL) ? 1 + isCRLF : 0;
		uint32_t newLinesLen = linesLen + lineLen + addnewline;

		// Add line to lines, concatenate \n character, if necessary

		if (newLinesLen >= linesCap)
		{
			// Resize lines array
			uint32_t newCap = (newLinesLen + 1) * 2;

			void * mem = realloc(lines, sizeof(wchar_t) * newCap);
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
				return checkRes_memError;
			}

			lines    = mem;
			linesCap = newCap;
		}

		// Copy line
		memcpy(lines + linesLen, line, sizeof(wchar_t) * lineLen);
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
			}
		}
		lines[linesLen] = L'\0';

		node = node->nextNode;
	}
	free(line);

	// Try to convert lines string to UTF-8
	char * utf8 = NULL;
	uint32_t utf8sz = 0;
	femto_convFromUnicode(lines, (int)linesLen + 1, &utf8, &utf8sz);

	// Free UTF-16 lines string
	free(lines);

	// Error-check conversion
	if (utf8 == NULL)
	{
		return checkRes_memError;
	}

	// Check if anything has changed, for that load original file again
	char * compFile = NULL;
	uint32_t compSize;
	if (femtoFile_readBytes(self, &compFile, &compSize) == NULL)
	{
		// Reading was successful
		bool areEqual = strncmp(utf8, compFile, (size_t)u32Min(utf8sz, compSize)) == 0;
		free(compFile);

		if (areEqual)
		{
			// Free all resources before returning
			free(utf8);
			self->bUnsaved = false;
			return checkRes_nothingNew;
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
	return checkRes_needsSaving;
}
int32_t femtoFile_write(femtoFile_t * restrict self)
{
	assert(self != NULL);
	char * utf8 = NULL;
	uint32_t utf8sz = 0;
	int32_t checkres = femtoFile_checkUnsaved(self, &utf8, &utf8sz);
	switch (checkres)
	{
	case checkRes_memError:
		return writeRes_memError;
	case checkRes_nothingNew:
		return writeRes_nothingNew;
	}
	
	// Make sure that any bugs won't slip through
	assert(utf8 != NULL);

	// Try to open file for writing
	if (femtoFile_open(self, NULL, true) == false)
	{
		return writeRes_openError;
	}

	if (self->bCanWrite == false)
	{
		femtoFile_close(self);
		return writeRes_writeError;
	}

	writeProfiler("Opened file for writing");

	// Try to write UTF-8 lines string to file
	DWORD dwWritten;

	// Write everything except the null terminator
	BOOL res = WriteFile(
		self->hFile,
		utf8,
		utf8sz - 1,
		&dwWritten,
		NULL
	);
	// Close file
	femtoFile_close(self);
	// Free utf8 string
	free(utf8);

	// Do error checking
	if (!res)
	{
		writeProfiler("Error writing to file");
		return writeRes_writeError;
	}
	else
	{
		self->bUnsaved = false;
		return (int32_t)dwWritten;
	}
}
void femtoFile_setConTitle(const femtoFile_t * restrict self)
{
	assert(self != NULL);
	wchar_t wndName[MAX_PATH];
	size_t fnamelen = wcslen(self->fileName);
	memcpy(wndName, self->fileName, fnamelen * sizeof(wchar_t));
	wcscpy_s(wndName + fnamelen, MAX_PATH - fnamelen, L" - femto");
	SetConsoleTitleW(wndName);
}


bool femtoFile_addNormalCh(femtoFile_t * restrict self, wchar_t ch, uint32_t tabWidth)
{
	assert(self != NULL);
	femtoLineNode_t * node = self->data.currentNode;
	assert(node != NULL);
	self->data.bTyped = true;
	
	writeProfiler("Add character %C", ch);

	if (!femtoLine_addChar(node, ch, tabWidth))
	{
		return false;
	}
	self->data.lastx = node->virtcurx;
	return true;
}
bool femtoFile_addSpecialCh(femtoFile_t * restrict self, uint32_t height, wchar_t ch, const femtoSettings_t * pset)
{
	assert(self != NULL);
	self->data.bTyped = true;
	femtoLineNode_t * lastcurnode = self->data.currentNode;

	switch (ch)
	{
	case VK_TAB:
	{
		wchar_t tch = pset->bTabsToSpaces ? L' ' : L'\t';
		if (!femtoFile_addNormalCh(self, tch, pset->tabWidth))
		{
			return false;
		}
		if (pset->bTabsToSpaces)
		{
			for (uint32_t i = 0, max = pset->tabWidth - (lastcurnode->virtcurx % pset->tabWidth); i < max; ++i)
			{
				if (femtoFile_addNormalCh(self, tch, pset->tabWidth) == false)
				{
					return false;
				}
			}
		}
		femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	}
	case VK_OEM_BACKTAB:
		// Check if there's 4 spaces before the caret
		if (femtoLine_checkAt(lastcurnode, -1, L" ", 1))
		{
			femtoFile_deleteBackward(self);
			--lastcurnode->virtcurx;
			uint32_t max = lastcurnode->virtcurx % pset->tabWidth;
			if (femtoLine_checkAt(lastcurnode, -(int32_t)max, pset->tabSpaceStr1, max))
			{
				for (uint32_t i = 0; i < max; ++i)
				{
					femtoFile_deleteBackward(self);
				}
			}
		}
		else if (femtoLine_checkAt(lastcurnode, -1, L"\t", 1))
		{
			femtoFile_deleteBackward(self);
		}
		// If there isn't, check if there's 4 spaces after the caret
		else if (femtoLine_checkAt(lastcurnode, 0, pset->tabSpaceStr1, pset->tabWidth))
		{
			for (int i = 0; i < pset->tabWidth; ++i)
			{
				femtoFile_deleteForward(self);
			}
		}
		else if (femtoLine_checkAt(lastcurnode, 0, L"\t", 1))
		{
			femtoFile_deleteForward(self);
		}
		femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_RETURN:	// Enter key
		femtoFile_addNewLine(self, pset->bTabsToSpaces, pset->tabWidth, pset->bAutoIndent);
		femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_BACK:	// Backspace
		femtoFile_deleteBackward(self);
		femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_DELETE:	// Delete
		femtoFile_deleteForward(self);
		femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case FEMTO_SHIFT_DEL:	// Shift+Delete
		if (lastcurnode->nextNode != NULL)
		{
			if (self->data.pcury == lastcurnode)
			{
				self->data.pcury = self->data.pcury->nextNode;
			}
			self->data.currentNode = lastcurnode->nextNode;
			self->data.currentNode->prevNode = lastcurnode->prevNode;
			if (lastcurnode->prevNode != NULL)
			{
				lastcurnode->prevNode->nextNode = self->data.currentNode;
			}

			// Destroy current line
			femtoLine_free(lastcurnode);
			self->data.bUpdateAll = true;
		}
		else if (lastcurnode->prevNode != NULL)
		{
			if (self->data.pcury == lastcurnode)
			{
				self->data.pcury = self->data.pcury->prevNode;
			}
			self->data.currentNode = lastcurnode->prevNode;
			self->data.currentNode->nextNode = NULL;

			femtoLine_free(lastcurnode);
			self->data.bUpdateAll = true;
		}
		break;
	case FEMTO_MOVELINE_UP:
		// Swap current line with previous if possible
		if (lastcurnode->prevNode != NULL)
		{
			femtoLine_swap(lastcurnode, self->data.currentNode->prevNode);
			self->data.currentNode = self->data.currentNode->prevNode;
			femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
			self->data.lastx       = self->data.currentNode->virtcurx;
			self->data.bUpdateAll   = true;
		}
		break;
	case FEMTO_MOVELINE_DOWN:
		// Swap current line with next if possible
		if (lastcurnode->nextNode != NULL)
		{
			femtoLine_swap(lastcurnode, self->data.currentNode->nextNode);
			self->data.currentNode = self->data.currentNode->nextNode;
			femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
			self->data.lastx       = self->data.currentNode->virtcurx;
			self->data.bUpdateAll   = true;
		}
		break;
	case VK_LEFT:	// Left arrow
		if (lastcurnode->curx > 0)
		{
			femtoLine_moveCursor(lastcurnode, -1);
		}
		else if (lastcurnode->prevNode != NULL)
		{
			self->data.currentNode = lastcurnode->prevNode;
			femtoLine_moveCursor(self->data.currentNode, (int32_t)self->data.currentNode->lineEndx);
		}
		femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_RIGHT:	// Right arrow
		if ((lastcurnode->curx + lastcurnode->freeSpaceLen) < lastcurnode->lineEndx)
		{
			femtoLine_moveCursor(lastcurnode, 1);
		}
		else if (lastcurnode->nextNode != NULL)
		{
			self->data.currentNode = self->data.currentNode->nextNode;
			femtoLine_moveCursor(self->data.currentNode, -(int32_t)self->data.currentNode->lineEndx);
		}
		femtoLine_calcVirtCursor(self->data.currentNode, pset->tabWidth);
		self->data.lastx = self->data.currentNode->virtcurx;
		break;
	case VK_UP:		// Up arrow
		femtoLine_moveCursorVert(&self->data.currentNode, -1);
		femtoLine_moveCursorAbs(self->data.currentNode, femtoLine_calcCursor(self->data.currentNode, self->data.lastx, pset->tabWidth));
		break;
	case VK_DOWN:	// Down arrow
		femtoLine_moveCursorVert(&self->data.currentNode, 1);
		femtoLine_moveCursorAbs(self->data.currentNode, femtoLine_calcCursor(self->data.currentNode, self->data.lastx, pset->tabWidth));
		break;
	case VK_PRIOR:	// Page up
		femtoLine_moveCursorVert(&self->data.currentNode, -(int32_t)height);
		femtoLine_moveCursorAbs(self->data.currentNode, femtoLine_calcCursor(self->data.currentNode, self->data.lastx, pset->tabWidth));
		break;
	case VK_NEXT:	// Page down
		femtoLine_moveCursorVert(&self->data.currentNode, (int32_t)height);
		femtoLine_moveCursorAbs(self->data.currentNode, femtoLine_calcCursor(self->data.currentNode, self->data.lastx, pset->tabWidth));
		break;
	case VK_END:
		femtoLine_moveCursor(lastcurnode, (int32_t)lastcurnode->lineEndx);
		femtoLine_calcVirtCursor(lastcurnode, pset->tabWidth);
		self->data.lastx = lastcurnode->virtcurx;
		break;
	case VK_HOME:
		femtoLine_moveCursor(lastcurnode, -(int32_t)lastcurnode->lineEndx);
		femtoLine_calcVirtCursor(lastcurnode, pset->tabWidth);
		self->data.lastx = lastcurnode->virtcurx;
		break;
	default:
		return false;
	}

	self->data.bUpdateAll |= (self->data.currentNode != lastcurnode) & pset->bRelLineNums;

	return true;
}

bool femtoFile_deleteForward(femtoFile_t * restrict self)
{
	assert(self != NULL);
	femtoLineNode_t * node = self->data.currentNode;
	if ((node->curx + node->freeSpaceLen) < node->lineEndx)
	{
		++node->freeSpaceLen;
		return true;
	}
	else if (node->nextNode != NULL)
	{
		self->data.bUpdateAll = true;
		return femtoLine_mergeNext(node, &self->data.pcury, &self->data.noLen);
	}
	else
	{
		return false;
	}
}
bool femtoFile_deleteBackward(femtoFile_t * restrict self)
{
	assert(self != NULL);
	femtoLineNode_t * node = self->data.currentNode;
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
		self->data.bUpdateAll = true;
		return femtoLine_mergeNext(self->data.currentNode, &self->data.pcury, &self->data.noLen);
	}
	else
	{
		return false;
	}
}
bool femtoFile_addNewLine(femtoFile_t * restrict self, bool tabsToSpaces, uint8_t tabWidth, bool autoIndent)
{
	assert(self != NULL);
	femtoLineNode_t * node = femtoLine_create(
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
	self->data.bUpdateAll = true;
	return true;
}

void femtoFile_updateCury(femtoFile_t * restrict self, uint32_t height)
{
	assert(self != NULL);
	if (self->data.pcury == NULL)
	{
		femtoLineNode_t * node = self->data.currentNode;
		for (uint32_t i = 0; i < height && node->prevNode != NULL; ++i)
		{
			node = node->prevNode;
		}
		self->data.pcury = node;
	}
	else
	{
		femtoLineNode_t * node = self->data.currentNode;
		for (uint32_t i = 0; i < height && node != NULL; ++i)
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
		femtoFile_updateCury(self, height);
	}
}
void femtoFile_scroll(femtoFile_t * restrict self, uint32_t height, int32_t deltaLines)
{
	assert(self != NULL);
	if (self->data.pcury == NULL)
	{
		femtoFile_updateCury(self, height);
	}

	if ((deltaLines != 0) && (self->data.pcury != NULL))
	{
		femtoLine_moveCursorVert(&self->data.pcury, deltaLines);
		self->data.bUpdateAll = true;
	}
}
void femtoFile_scrollHor(femtoFile_t * restrict self, uint32_t width, int32_t deltaCh)
{
	assert(self != NULL);
	if ((deltaCh < 0) && ((uint32_t)-deltaCh <= self->data.curx))
	{
		self->data.curx -= (uint32_t)-deltaCh;
	}
	else if (deltaCh > 0)
	{
		uint32_t curx = self->data.curx + (uint32_t)deltaCh, total = self->data.currentNode->lineEndx - self->data.currentNode->freeSpaceLen;
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


void femtoFile_destroy(femtoFile_t * restrict self)
{
	assert(self != NULL);
	femtoFile_close(self);
	femtoFile_clearLines(self);

	if (self->fileName != NULL)
	{
		free(self->fileName);
		self->fileName = NULL;
	}
}

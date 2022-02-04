#include "femtoFile.h"
#include "femto.h"


void femtoFile_reset(femtoFile_t * restrict self)
{
	assert(self != NULL);
	(*self) = (femtoFile_t){
		.fileName = NULL,
		.hFile    = INVALID_HANDLE_VALUE,
		.canWrite = false,
		.eolSeq   = EOL_not,
		.data     = {
			.firstNode   = NULL,
			.currentNode = NULL,
			.pcury       = NULL,
			.curx        = 0,
			.lastx       = 0,
			.typed       = false,
			.updateAll   = false
		}
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
bool femtoFile_open(femtoFile_t * restrict self, const wchar_t * restrict fileName, bool writemode)
{
	assert(self != NULL);
	if (fileName == NULL)
	{
		fileName = self->fileName;
	}

	// try to open file
	self->hFile = CreateFileW(
		fileName,
		writemode ? GENERIC_WRITE : GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		writemode ? CREATE_ALWAYS : OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (self->hFile == INVALID_HANDLE_VALUE)
	{
		self->canWrite = false;
		return false;
	}
	else
	{
		self->canWrite = writemode;
		self->fileName = fileName;
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
		femtoLine_destroy(node);
		node = next;
	}
}
const wchar_t * femtoFile_readBytes(femtoFile_t * restrict self, char ** bytes, uint32_t * bytesLen)
{
	assert(self != NULL);
	assert(bytes != NULL);
	assert(bytesLen != NULL && "Pointer to length variable is mandatory!");
	if (femtoFile_open(self, NULL, false) == false)
	{
		return L"File opening error!";
	}
	DWORD fileSize = GetFileSize(self->hFile, NULL);
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
		self->hFile,
		*bytes,
		fileSize,
		NULL,
		NULL
	);
	femtoFile_close(self);
	if (!readFileRes)
	{
		return L"File read error!";
	}
	// Add null terminator
	(*bytes)[fileSize] = '\0';

	return NULL;
}
const wchar_t * femtoFile_read(femtoFile_t * restrict self, uint8_t tabWidth)
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
	writeProfiler("femtoFile_read", "Converted %u bytes of character to %u UTF-16 characters.", size, chars);
	writeProfiler("femtoFile_read", "File UTF-16 contents \"%S\"", utf16);

	femto_tabsToSpaces(&utf16, &chars, tabWidth);

	// Save lines to structure
	wchar_t ** lines = NULL;
	uint32_t numLines = femto_strnToLines(utf16, chars, &lines, &self->eolSeq);
	if (lines == NULL)
	{
		free(utf16);
		return L"Line reading error!";
	}
	writeProfiler("femtoFile_read", "Total of %u lines", numLines);
	for (uint32_t i = 0; i < numLines; ++i)
	{
		writeProfiler("femtoFile_read", "Line %d: \"%S\"", i, lines[i]);
	}

	femtoFile_clearLines(self);
	if (numLines == 0)
	{
		self->data.firstNode = femtoLine_create(NULL, NULL);
		if (self->data.firstNode == NULL)
		{
			free(lines);
			free(utf16);
			return L"Line creation error!";
		}
	}
	else
	{
		self->data.firstNode = femtoLine_createText(NULL, NULL, lines[0], -1);
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
		femtoLineNode_t * node = femtoLine_createText(self->data.currentNode, NULL, lines[i], -1);
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
int32_t femtoFile_write(femtoFile_t * restrict self, uint8_t tabWidth)
{
	assert(self != NULL);
	// Generate lines
	wchar_t * lines = NULL, * line = NULL;
	uint32_t linesCap = 0, linesLen = 0, lineCap = 0;

	femtoLineNode_t * node = self->data.firstNode;

	const uint8_t eolSeq = self->eolSeq;
	bool isCRLF = (self->eolSeq == EOL_CRLF);

	while (node != NULL)
	{
		if (femtoLine_getText(node, &line, &lineCap) == false)
		{
			writeProfiler("femtoFile_write", "Failed to fetch line!");
			if (line != NULL)
			{
				free(line);
			}
			if (lines != NULL)
			{
				free(lines);
			}
			return writeRes_memError;
		}

		uint32_t lineLen = (uint32_t)wcsnlen(line, lineCap);

		writeProfiler("femtoFile_write", "Got line with size of %u characters. Line contents: \"%S\"", lineLen, line);

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
				return writeRes_memError;
			}

			lines    = mem;
			linesCap = newCap;

			writeProfiler("femtoFile_write", "Resized line string. New cap, length is %u, %u bytes.", linesCap, linesLen);
		}

		// Copy line
		memcpy(lines + linesLen, line, sizeof(wchar_t) * lineLen);
		linesLen = newLinesLen;

		if (addnewline)
		{
			switch (eolSeq)
			{
			case EOL_CR:
				lines[linesLen - 1] = L'\r';
				break;
			case EOL_LF:
				lines[linesLen - 1] = L'\n';
				break;
			case EOL_CRLF:
				lines[linesLen - 2] = L'\r';
				lines[linesLen - 1] = L'\n';
				break;
			}
		}
		lines[linesLen] = L'\0';

		node = node->nextNode;
	}
	free(line);

	writeProfiler("femtoFile_write", "All file contents (%u): \"%S\"", linesLen, lines);

	// Convert spaces to tabs
	femto_spacesToTabs(&lines, &linesLen, tabWidth);

	// Try to convert lines string to UTF-8
	char * utf8 = NULL;
	uint32_t utf8sz = 0;
	femto_convFromUnicode(lines, (int)linesLen + 1, &utf8, &utf8sz);

	// Free UTF-16 lines string
	free(lines);

	// Error-check conversion
	if (utf8 == NULL)
	{
		return writeRes_memError;
	}

	writeProfiler("femtoFile_write", "Converted UTF-16 string to UTF-8 string");
	writeProfiler("femtoFile_write", "UTF-8 contents: \"%s\"", utf8);

	// Check if anything has changed, for that load original file again
	char * compFile = NULL;
	uint32_t compSize;
	if (femtoFile_readBytes(self, &compFile, &compSize) == NULL)
	{
		// Reading was successful
		writeProfiler("femtoFile_write", "Comparing strings: \"%s\" and \"%s\" with sizes %u and %u", utf8, compFile, utf8sz, compSize);

		bool areEqual = strncmp(utf8, compFile, (size_t)u32Min(utf8sz, compSize)) == 0;
		free(compFile);

		writeProfiler("femtoFile_write", "The strings in question are %s", areEqual ? "equal" : "not equal");

		if (areEqual)
		{
			// Free all resources before returning
			free(utf8);
			return writeRes_nothingNew;
		}
	}

	// Try to open file for writing
	if (femtoFile_open(self, NULL, true) == false)
	{
		return writeRes_openError;
	}

	if (self->canWrite == false)
	{
		femtoFile_close(self);
		return writeRes_writeError;
	}

	writeProfiler("femtoFile_write", "Opened file for writing");

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
		writeProfiler("femtoFile_write", "Error writing to file");
		return writeRes_writeError;
	}
	else
	{
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


bool femtoFile_addNormalCh(femtoFile_t * restrict self, wchar_t ch)
{
	assert(self != NULL);
	assert(self->data.currentNode != NULL);
	self->data.typed = true;
	
	writeProfiler("femtoFile_addNormalCh", "Add character %C", ch);
	femtoLineNode_t * node = self->data.currentNode;

	if ((node->freeSpaceLen == 0) && !femtoLine_realloc(node))
	{
		return false;
	}

	writeProfiler("femtoFile_addNormalCh", "curx: %u, free: %u, end: %u", node->curx, node->freeSpaceLen, node->lineEndx);

	node->line[node->curx] = ch;
	++node->curx;
	--node->freeSpaceLen;

	self->data.lastx = self->data.currentNode->curx;
	return true;
}
bool femtoFile_addSpecialCh(femtoFile_t * restrict self, uint32_t height, wchar_t ch, const femtoSettings_t * pset)
{
	assert(self != NULL);
	self->data.typed = true;
	switch (ch)
	{
	case VK_TAB:
	{
		wchar_t tch = pset->tabsToSpaces ? L' ' : L'\t';
		if (!femtoFile_addNormalCh(self, tch))
		{
			return false;
		}
		for (int i = 1; (i < pset->tabWidth) && (self->data.currentNode->curx % pset->tabWidth); ++i)
		{
			if (femtoFile_addNormalCh(self, tch) == false)
			{
				return false;
			}
		}
		self->data.lastx = self->data.currentNode->curx;
		break;
	}
	case VK_OEM_BACKTAB:
		// Check if there's 4 spaces before the caret
		if (femtoFile_checkLineAt(self, -pset->tabWidth, pset->tabSpaceStr1, pset->tabWidth) ||
		    femtoFile_checkLineAt(self, -pset->tabWidth, pset->tabSpaceStr2, pset->tabWidth)
		)
		{
			for (int i = 0; i < pset->tabWidth; ++i)
			{
				femtoFile_deleteBackward(self);
			}
		}
		// If there isn't, check if there's 4 spaces after the caret
		else if (femtoFile_checkLineAt(self, 0, pset->tabSpaceStr1, pset->tabWidth) ||
		         femtoFile_checkLineAt(self, 0, pset->tabSpaceStr2, pset->tabWidth)
		)
		{
			for (int i = 0; i < pset->tabWidth; ++i)
			{
				femtoFile_deleteForward(self);
			}
		}
		self->data.lastx = self->data.currentNode->curx;
		break;
	case VK_RETURN:	// Enter key
		femtoFile_addNewLine(self);
		self->data.lastx = self->data.currentNode->curx;
		break;
	case VK_BACK:	// Backspace
		femtoFile_deleteBackward(self);
		self->data.lastx = self->data.currentNode->curx;
		break;
	case VK_DELETE:	// Delete
		femtoFile_deleteForward(self);
		self->data.lastx = self->data.currentNode->curx;
		break;
	case FEMTO_SHIFT_DEL:	// Shift+Delete
		if (self->data.currentNode->nextNode != NULL)
		{
			if (self->data.pcury == self->data.currentNode)
			{
				self->data.pcury = self->data.pcury->nextNode;
			}
			femtoLineNode_t * node = self->data.currentNode;
			self->data.currentNode = node->nextNode;
			self->data.currentNode->prevNode = node->prevNode;
			if (node->prevNode != NULL)
			{
				node->prevNode->nextNode = self->data.currentNode;
			}

			// Destroy current line
			femtoLine_destroy(node);
			self->data.updateAll = true;
		}
		break;
	case FEMTO_MOVELINE_UP:
		// Swap current line with previous if possible
		if (self->data.currentNode->prevNode != NULL)
		{
			femtoLine_swap(self->data.currentNode, self->data.currentNode->prevNode);
			self->data.currentNode = self->data.currentNode->prevNode;
			self->data.lastx       = self->data.currentNode->curx;
			self->data.updateAll   = true;
		}
		break;
	case FEMTO_MOVELINE_DOWN:
		// Swap current line with next if possible
		if (self->data.currentNode->nextNode != NULL)
		{
			femtoLine_swap(self->data.currentNode, self->data.currentNode->nextNode);
			self->data.currentNode = self->data.currentNode->nextNode;
			self->data.lastx       = self->data.currentNode->curx;
			self->data.updateAll   = true;
		}
		break;
	case VK_LEFT:	// Left arrow
		if (self->data.currentNode->curx > 0)
		{
			femtoLine_moveCursor(self->data.currentNode, -1);
		}
		else if (self->data.currentNode->prevNode != NULL)
		{
			self->data.currentNode = self->data.currentNode->prevNode;
			femtoLine_moveCursor(self->data.currentNode, (int32_t)self->data.currentNode->lineEndx);
		}
		self->data.lastx = self->data.currentNode->curx;
		break;
	case VK_RIGHT:	// Right arrow
		if ((self->data.currentNode->curx + self->data.currentNode->freeSpaceLen) < self->data.currentNode->lineEndx)
		{
			femtoLine_moveCursor(self->data.currentNode, 1);
		}
		else if (self->data.currentNode->nextNode != NULL)
		{
			self->data.currentNode = self->data.currentNode->nextNode;
			femtoLine_moveCursor(self->data.currentNode, -(int32_t)self->data.currentNode->lineEndx);
		}
		self->data.lastx = self->data.currentNode->curx;
		break;
	case VK_UP:		// Up arrow
		femtoLine_moveCursorVert(&self->data.currentNode, -1);
		femtoLine_moveCursorAbs(self->data.currentNode, self->data.lastx);
		break;
	case VK_DOWN:	// Down arrow
		femtoLine_moveCursorVert(&self->data.currentNode, 1);
		femtoLine_moveCursorAbs(self->data.currentNode, self->data.lastx);
		break;
	case VK_PRIOR:	// Page up
		femtoLine_moveCursorVert(&self->data.currentNode, -(int32_t)height);
		femtoLine_moveCursorAbs(self->data.currentNode, self->data.lastx);
		break;
	case VK_NEXT:	// Page down
		femtoLine_moveCursorVert(&self->data.currentNode, (int32_t)height);
		femtoLine_moveCursorAbs(self->data.currentNode, self->data.lastx);
		break;
	case VK_END:
		femtoLine_moveCursor(self->data.currentNode, (int32_t)self->data.currentNode->lineEndx);
		break;
	case VK_HOME:
		femtoLine_moveCursor(self->data.currentNode, -(int32_t)self->data.currentNode->lineEndx);
		break;
	default:
		return false;
	}

	return true;
}

bool femtoFile_checkLineAt(const femtoFile_t * restrict self, int32_t maxdelta, const wchar_t * restrict string, uint32_t maxString)
{
	assert(self != NULL);
	assert(string != NULL);
	const femtoLineNode_t * restrict node = self->data.currentNode;
	if (node == NULL)
	{
		return false;
	}

	int32_t idx = (int32_t)node->curx + maxdelta, i = 0, m = (int32_t)maxString;
	if (idx < 0)
	{
		return false;
	}
	for (; idx < (int32_t)node->lineEndx && i < m && *string != '\0';)
	{
		if (idx == (int32_t)node->curx)
		{
			idx += (int32_t)node->freeSpaceLen;
			continue;
		}
		
		if (node->line[idx] != *string)
		{
			return false;
		}

		++string;
		++i;
		++idx;
	}
	if (*string != '\0' && i < m)
	{
		return false;
	}

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
		return femtoLine_mergeNext(node, &self->data.pcury);
		self->data.updateAll = true;
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
		return femtoLine_mergeNext(self->data.currentNode, &self->data.pcury);
		self->data.updateAll = true;
	}
	else
	{
		return false;
	}
}
bool femtoFile_addNewLine(femtoFile_t * restrict self)
{
	assert(self != NULL);
	femtoLineNode_t * node = femtoLine_create(self->data.currentNode, self->data.currentNode->nextNode);
	if (node == NULL)
	{
		return false;
	}

	self->data.currentNode->nextNode = node;
	self->data.currentNode = node;
	self->data.updateAll = true;
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
		self->data.updateAll = true;
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
	self->data.updateAll = true;
}


void femtoFile_destroy(femtoFile_t * restrict self)
{
	assert(self != NULL);
	femtoFile_close(self);
	femtoFile_clearLines(self);
}

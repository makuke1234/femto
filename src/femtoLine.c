#include "femtoLine.h"


femtoLineNode_t * femtoLine_create(femtoLineNode_t * restrict curnode, femtoLineNode_t * restrict nextnode, bool tabsToSpaces, bool autoIndent, uint8_t * restrict noLen)
{
	assert(noLen != NULL);
	femtoLineNode_t * node = malloc(sizeof(femtoLineNode_t));
	if (node == NULL)
	{
		return NULL;
	}

	wchar_t tch = tabsToSpaces ? L' ' : L'\t';

	uint32_t space = 0;
	if (autoIndent)
	{
		for (uint32_t i = 0; i < curnode->lineEndx;)
		{
			if ((i == curnode->curx) && (curnode->freeSpaceLen > 0))
			{
				i += curnode->freeSpaceLen;
				continue;
			}

			if (curnode->line[i] != L' ' && curnode->line[i] != L'\t')
			{
				break;
			}

			++space;
			++i;
		}
	}

	if (curnode != NULL)
	{
		// Create normal empty line
		if ((curnode->curx + curnode->freeSpaceLen) == curnode->lineEndx)
		{
			node->line = malloc(sizeof(wchar_t) * (FEMTO_LNODE_DEFAULT_FREE + space));
			if (node->line == NULL)
			{
				free(node);
				return NULL;
			}
			node->lineEndx = FEMTO_LNODE_DEFAULT_FREE + space;
			for (uint32_t i = 0; i < space; ++i)
			{
				node->line[i] = tch;
			}
		}
		// Copy contents after cursor to this line
		else
		{
			uint32_t contStart = curnode->curx + curnode->freeSpaceLen, contLen = curnode->lineEndx - contStart;
			node->lineEndx = contLen + FEMTO_LNODE_DEFAULT_FREE + space;
			node->line = malloc(sizeof(wchar_t) * node->lineEndx);
			if (node->line == NULL)
			{
				free(node);
				return NULL;
			}
			for (uint32_t i = 0; i < space; ++i)
			{
				node->line[i] = tch;
			}
			memcpy(node->line + FEMTO_LNODE_DEFAULT_FREE + space, curnode->line + contStart, sizeof(wchar_t) * contLen);
			curnode->freeSpaceLen += contLen;
		}
	}

	node->curx = space;
	node->freeSpaceLen = FEMTO_LNODE_DEFAULT_FREE;
	node->prevNode = curnode;
	node->nextNode = nextnode;
	node->lineNumber = 1;
	if (curnode != NULL)
	{
		curnode->nextNode  = node;
		node->lineNumber = curnode->lineNumber + 1;
	}
	if (nextnode != NULL)
	{
		nextnode->prevNode = node;
	}
	femtoLine_updateLineNumbers(node, node->lineNumber, noLen);
	node->virtcurx = 0;

	return node;
}

femtoLineNode_t * femtoLine_createText(
	femtoLineNode_t * restrict curnode,
	femtoLineNode_t * restrict nextnode,
	const wchar_t * restrict lineText,
	int32_t mText,
	uint8_t * restrict noLen
)
{
	assert(noLen != NULL);

	uint32_t maxText = (mText == -1) ? (uint32_t)wcslen(lineText) : (uint32_t)mText;

	femtoLineNode_t * node = malloc(sizeof(femtoLineNode_t));
	if (node == NULL)
	{
		return NULL;
	}

	node->lineEndx = maxText + FEMTO_LNODE_DEFAULT_FREE;
	node->line = malloc(sizeof(wchar_t) * node->lineEndx);
	if (node->line == NULL)
	{
		free(node);
		return NULL;
	}
	
	memcpy(node->line, lineText, sizeof(wchar_t) * maxText);

	node->curx = maxText;
	node->freeSpaceLen = FEMTO_LNODE_DEFAULT_FREE;

	node->prevNode = curnode;
	node->nextNode = nextnode;
	node->lineNumber = 1;
	if (curnode != NULL)
	{
		curnode->nextNode  = node;
		node->lineNumber = curnode->lineNumber + 1;
	}
	if (nextnode != NULL)
	{
		nextnode->prevNode = node;
	}
	femtoLine_updateLineNumbers(node, node->lineNumber, noLen);
	node->virtcurx = 0;
	return node;
}

bool femtoLine_getText(const femtoLineNode_t * restrict self, wchar_t ** restrict text, uint32_t * restrict tarrsz)
{
	assert(self != NULL);
	assert(text != NULL);

	uint32_t totalLen = self->lineEndx - self->freeSpaceLen + 1;
	writeProfiler("femtoLine_getText", "Total length: %u characters", totalLen);

	if (tarrsz != NULL && *tarrsz < totalLen)
	{
		wchar_t * mem = realloc(*text, sizeof(wchar_t) * totalLen);
		if (mem == NULL)
		{
			return false;
		}
		*text   = mem;
		*tarrsz = totalLen;
	}
	else if (tarrsz == NULL)
	{
		*text = malloc(sizeof(wchar_t) * totalLen);
		if (*text == NULL)
		{
			return false;
		}
	}

	wchar_t * t = *text;
	for (uint32_t i = 0; i < self->lineEndx;)
	{
		if (i == self->curx)
		{
			i += self->freeSpaceLen;
			continue;
		}

		*t = self->line[i];
		++t;
		++i;
	}
	*t = L'\0';

	return true;
}

bool femtoLine_realloc(femtoLineNode_t * restrict self)
{
	assert(self != NULL);
	if (self->freeSpaceLen == FEMTO_LNODE_DEFAULT_FREE)
	{
		writeProfiler("femtoLine_realloc", "Reallocation succeeded");
		return true;
	}
	writeProfiler("femtoLine_realloc", "Reallocating line");
	uint32_t totalLen = self->lineEndx - self->freeSpaceLen;
	void * newmem = realloc(self->line, sizeof(wchar_t) * (totalLen + FEMTO_LNODE_DEFAULT_FREE));
	if (newmem == NULL)
	{
		return false;
	}
	self->line = newmem;
	if (self->curx < totalLen)
	{
		memmove(
			self->line + self->curx + FEMTO_LNODE_DEFAULT_FREE,
			self->line + self->curx + self->freeSpaceLen,
			sizeof(wchar_t) * (totalLen - self->curx)
		);
	}

	self->lineEndx     = totalLen + FEMTO_LNODE_DEFAULT_FREE;
	self->freeSpaceLen = FEMTO_LNODE_DEFAULT_FREE;

	writeProfiler("femtoLine_realloc", "Reallocation succeeded");
	return true;
}

bool femtoLine_mergeNext(femtoLineNode_t * restrict self, femtoLineNode_t ** restrict ppcury, uint8_t * restrict noLen)
{
	assert(self != NULL);
	assert(ppcury != NULL);
	if (self->nextNode == NULL)
	{
		return false;
	}
	
	femtoLineNode_t * restrict n = self->nextNode;

	if (*ppcury == n)
	{
		*ppcury = self;
	}

	// Allocate more memory for first line
	void * linemem = realloc(
		self->line,
		sizeof(wchar_t) * (self->lineEndx - self->freeSpaceLen + n->lineEndx - n->freeSpaceLen + FEMTO_LNODE_DEFAULT_FREE)
	);
	if (linemem == NULL)
	{
		return false;
	}
	self->line = linemem;

	// Move cursor to end, if needed
	femtoLine_moveCursor(self, (int32_t)self->lineEndx);
	femtoLine_moveCursor(n,    (int32_t)n->lineEndx);

	self->freeSpaceLen = FEMTO_LNODE_DEFAULT_FREE;
	self->lineEndx     = self->curx + n->curx + FEMTO_LNODE_DEFAULT_FREE;

	memcpy(self->line + self->curx + self->freeSpaceLen, n->line, sizeof(wchar_t) * n->curx);
	self->nextNode = n->nextNode;
	if (self->nextNode != NULL)
	{
		self->nextNode->prevNode = self;
	}
	femtoLine_free(n); 

	femtoLine_updateLineNumbers(self->nextNode, self->lineNumber + 1, noLen);

	return true;
}

void femtoLine_moveCursor(femtoLineNode_t * restrict self, int32_t delta)
{
	assert(self != NULL);
	if (delta < 0)
	{
		for (uint32_t idx = self->curx + self->freeSpaceLen; delta < 0 && self->curx > 0; ++delta)
		{
			--idx;
			--self->curx;
			self->line[idx] = self->line[self->curx];
		}
	}
	else
	{
		for (uint32_t total = self->lineEndx - self->freeSpaceLen, idx = self->curx + self->freeSpaceLen; delta > 0 && self->curx < total; --delta)
		{
			self->line[self->curx] = self->line[idx];
			++idx;
			++self->curx;
		}
	}
}
void femtoLine_moveCursorAbs(femtoLineNode_t * restrict self, uint32_t curx)
{
	assert(self != NULL);
	if (curx < self->curx)
	{
		for (uint32_t idx = self->curx + self->freeSpaceLen; curx != self->curx && self->curx > 0;)
		{
			--idx;
			--self->curx;
			self->line[idx] = self->line[self->curx];
		}
	}
	else
	{
		for (uint32_t total = self->lineEndx - self->freeSpaceLen, idx = self->curx + self->freeSpaceLen; curx != self->curx && self->curx < total;)
		{
			self->line[self->curx] = self->line[idx];
			++idx;
			++self->curx;
		}
	}
}
void femtoLine_moveCursorVert(femtoLineNode_t ** restrict self, int32_t delta)
{
	assert(self != NULL);
	assert(*self != NULL);

	femtoLineNode_t * node = *self;
	if (delta < 0)
	{
		for (; delta != 0 && node->prevNode != NULL; ++delta)
		{
			node = node->prevNode;
		}
	}
	else
	{
		for (; delta != 0 && node->nextNode != NULL; --delta)
		{
			node = node->nextNode;
		}
	}
	*self = node;
}
void femtoLine_calcVirtCursor(femtoLineNode_t * restrict self, uint32_t tabWidth)
{
	assert(self != NULL);
	assert(tabWidth > 0);
	self->virtcurx = 0;
	for (uint32_t i = 0; i < self->curx; ++i)
	{
		self->virtcurx += (self->line[i] == L'\t') ? tabWidth - (self->virtcurx % tabWidth) : 1;
	}
}
uint32_t femtoLine_calcCursor(const femtoLineNode_t * restrict self, uint32_t virtcur, uint32_t tabWidth)
{
	assert(self != NULL);
	assert(tabWidth > 0);

	uint32_t curx = 0;
	for (uint32_t i = 0; (i < virtcur) && (curx < self->lineEndx);)
	{
		if ((curx == self->curx) && (self->freeSpaceLen > 0))
		{
			curx += self->freeSpaceLen;
			continue;
		}
		i += (self->line[curx] == L'\t') ? tabWidth - (i % tabWidth) : 1;
		++curx;
	}

	writeProfiler("femtoLine_calcCursor", "virtual: %d curx: %d", virtcur, curx);
	return curx;
}

void femtoLine_swap(femtoLineNode_t * restrict node1, femtoLineNode_t * restrict node2)
{
	assert(node1 != NULL);
	assert(node2 != NULL);

	femtoLineNode_t temp = *node1;
	node1->line         = node2->line;
	node1->lineEndx     = node2->lineEndx;
	node1->curx         = node2->curx;
	node1->freeSpaceLen = node2->freeSpaceLen;
	node1->virtcurx     = node2->virtcurx;

	node2->line         = temp.line;
	node2->lineEndx     = temp.lineEndx;
	node2->curx         = temp.curx;
	node2->freeSpaceLen = temp.freeSpaceLen;
	node2->virtcurx     = temp.virtcurx;
}

void femtoLine_updateLineNumbers(femtoLineNode_t * restrict startnode, uint32_t startLno, uint8_t * restrict noLen)
{
	femtoLineNode_t * prevnode = NULL;
	while (startnode != NULL)
	{
		startnode->lineNumber = startLno;
		++startLno;

		prevnode  = startnode;
		startnode = startnode->nextNode;
	}
	if ((prevnode != NULL) && (prevnode->lineNumber > 0))
	{
		*noLen = (uint8_t)log10((double)prevnode->lineNumber) + 1;
	}
	else
	{
		*noLen = 1;
	}
}

void femtoLine_free(femtoLineNode_t * restrict self)
{
	assert(self != NULL);
	if (self->line != NULL)
	{
		free(self->line);
		self->line = NULL;
	}
	free(self);
}

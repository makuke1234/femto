#include "fLine.h"
#include "fSyntax.h"

void fLine_init(fLine_t * restrict self)
{
	assert(self != NULL);
	*self = (fLine_t){
		.line         = NULL,
		.lineEndx     = 0,
		.curx         = 0,
		.freeSpaceLen = 0,
		.prevNode     = NULL,
		.nextNode     = NULL,
		.virtcurx     = 0,

		.lineNumber   = 1,

		.syntax       = NULL,
		.userValue = false
	};
}
fLine_t * fLine_create(
	fLine_t * restrict curnode,
	fLine_t * restrict nextnode,
	bool tabsToSpaces,
	uint8_t tabWidth,
	bool autoIndent,
	uint8_t * restrict noLen
)
{
	assert(noLen != NULL);
	fLine_t * node = malloc(sizeof(fLine_t));
	if (node == NULL)
	{
		return NULL;
	}

	wchar tch = tabsToSpaces ? L' ' : L'\t';

	u32 space = 0;
	if (curnode != NULL)
	{
		if (autoIndent)
		{
			for (u32 i = 0; i < curnode->lineEndx;)
			{
				if ((i == curnode->curx) && (curnode->freeSpaceLen > 0))
				{
					i += curnode->freeSpaceLen;
					continue;
				}

				if ((curnode->line[i] != L' ') && (curnode->line[i] != L'\t'))
				{
					break;
				}
				++space;
				if (curnode->line[i] == L'\t')
				{
					space += tabWidth - (space % tabWidth);
				}

				++i;
			}
			if (!tabsToSpaces)
			{
				space /= tabWidth;
			}
		}

		
		// Create normal empty line
		if ((curnode->curx + curnode->freeSpaceLen) == curnode->lineEndx)
		{
			node->line = malloc(sizeof(wchar) * (FEMTO_LNODE_DEFAULT_FREE + space));
			if (node->line == NULL)
			{
				free(node);
				return NULL;
			}
			node->lineEndx = FEMTO_LNODE_DEFAULT_FREE + space;
			for (u32 i = 0; i < space; ++i)
			{
				node->line[i] = tch;
			}
		}
		// Copy contents after cursor to this line
		else
		{
			u32 contStart = curnode->curx + curnode->freeSpaceLen, contLen = curnode->lineEndx - contStart;
			node->lineEndx = contLen + FEMTO_LNODE_DEFAULT_FREE + space;
			node->line = malloc(sizeof(wchar) * node->lineEndx);
			if (node->line == NULL)
			{
				free(node);
				return NULL;
			}
			for (u32 i = 0; i < space; ++i)
			{
				node->line[i] = tch;
			}
			memcpy(node->line + FEMTO_LNODE_DEFAULT_FREE + space, curnode->line + contStart, sizeof(wchar) * contLen);
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
	fLine_updateLineNumbers(node, node->lineNumber, noLen);
	node->virtcurx = 0;

	node->syntax = NULL;
	node->userValue = false;

	return node;
}

fLine_t * fLine_createText(
	fLine_t * restrict curnode,
	fLine_t * restrict nextnode,
	const wchar * restrict lineText,
	i32 mText,
	uint8_t * restrict noLen
)
{
	assert(noLen != NULL);

	u32 maxText = (mText == -1) ? (u32)wcslen(lineText) : (u32)mText;

	fLine_t * node = malloc(sizeof(fLine_t));
	if (node == NULL)
	{
		return NULL;
	}

	node->lineEndx = maxText + FEMTO_LNODE_DEFAULT_FREE;
	node->line = malloc(sizeof(wchar) * node->lineEndx);
	if (node->line == NULL)
	{
		free(node);
		return NULL;
	}
	
	memcpy(node->line, lineText, sizeof(wchar) * maxText);

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
	fLine_updateLineNumbers(node, node->lineNumber, noLen);
	node->virtcurx = 0;

	node->syntax = NULL;
	node->userValue = false;

	return node;
}

bool fLine_getText(const fLine_t * restrict self, wchar ** restrict text, u32 * restrict tarrsz)
{
	assert(self != NULL);
	assert(text != NULL);

	u32 totalLen = self->lineEndx - self->freeSpaceLen + 1;
	fProf_write("Total length: %u characters", totalLen);

	if (tarrsz != NULL && *tarrsz < totalLen)
	{
		wchar * mem = realloc(*text, sizeof(wchar) * totalLen);
		if (mem == NULL)
		{
			return false;
		}
		*text   = mem;
		*tarrsz = totalLen;
	}
	else if (tarrsz == NULL)
	{
		*text = malloc(sizeof(wchar) * totalLen);
		if (*text == NULL)
		{
			return false;
		}
	}

	wchar * t = *text;
	for (u32 i = 0; i < self->lineEndx;)
	{
		if ((i == self->curx) && (self->freeSpaceLen > 0))
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
void fLine_getTextLim(const fLine_t * restrict self, wchar * restrict text, u32 maxLen)
{
	assert(self != NULL);
	assert(text != NULL);
	assert(maxLen > 0);

	u32 len = 0;
	for (u32 i = 0; (i < self->lineEndx) && (len < (maxLen - 1));)
	{
		if ((i == self->curx) && (self->freeSpaceLen > 0))
		{
			i += self->freeSpaceLen;
			continue;
		}

		text[len] = self->line[i];
		++len;
		++i;
	}

	text[len] = L'\0';
}

bool fLine_realloc(fLine_t * restrict self)
{
	assert(self != NULL);
	if (self->freeSpaceLen == FEMTO_LNODE_DEFAULT_FREE)
	{
		return true;
	}
	u32 totalLen = self->lineEndx - self->freeSpaceLen;
	vptr newmem = realloc(self->line, sizeof(wchar) * (totalLen + FEMTO_LNODE_DEFAULT_FREE));
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
			sizeof(wchar) * (totalLen - self->curx)
		);
	}

	self->lineEndx     = totalLen + FEMTO_LNODE_DEFAULT_FREE;
	self->freeSpaceLen = FEMTO_LNODE_DEFAULT_FREE;

	free(self->syntax);
	self->syntax = NULL;

	return true;
}

bool fLine_addChar(fLine_t * restrict self, wchar ch, u32 tabWidth)
{
	assert(self != NULL);
	assert(ch   != L'\0');

	if ((self->freeSpaceLen == 0) && !fLine_realloc(self))
	{
		return false;
	}

	self->line[self->curx] = ch;
	++self->curx;
	--self->freeSpaceLen;

	fLine_calcVirtCursor(self, tabWidth);

	return true;
}
bool fLine_checkAt(const fLine_t * restrict node, i32 maxdelta, const wchar * restrict string, u32 maxString)
{
	assert(string != NULL);
	if (node == NULL)
	{
		return false;
	}

	i32 idx = (i32)node->curx + maxdelta, i = 0, m = (i32)maxString;
	if (idx < 0)
	{
		return false;
	}
	for (; idx < (i32)node->lineEndx && i < m && *string != '\0';)
	{
		if (idx == (i32)node->curx)
		{
			idx += (i32)node->freeSpaceLen;
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

bool fLine_mergeNext(fLine_t * restrict self, fLine_t ** restrict ppcury, uint8_t * restrict noLen)
{
	assert(self != NULL);
	assert(ppcury != NULL);
	if (self->nextNode == NULL)
	{
		return false;
	}
	
	fLine_t * restrict n = self->nextNode;
	*ppcury = (*ppcury == n) ? self : *ppcury;

	// Allocate more memory for first line
	vptr linemem = realloc(
		self->line,
		sizeof(wchar) * (self->lineEndx - self->freeSpaceLen + n->lineEndx - n->freeSpaceLen + FEMTO_LNODE_DEFAULT_FREE)
	);
	if (linemem == NULL)
	{
		return false;
	}
	self->line = linemem;

	// Move cursor to end, if needed
	fLine_moveCursor(self, (i32)self->lineEndx);
	fLine_moveCursor(n,    (i32)n->lineEndx);

	self->freeSpaceLen = FEMTO_LNODE_DEFAULT_FREE;
	self->lineEndx     = self->curx + n->curx + FEMTO_LNODE_DEFAULT_FREE;

	memcpy(self->line + self->curx + self->freeSpaceLen, n->line, sizeof(wchar) * n->curx);
	self->nextNode = n->nextNode;
	if (self->nextNode != NULL)
	{
		self->nextNode->prevNode = self;
	}
	fLine_free(n); 

	fLine_updateLineNumbers(self->nextNode, self->lineNumber + 1, noLen);

	free(self->syntax);
	self->syntax = NULL;

	return true;
}

void fLine_moveCursor(fLine_t * restrict self, i32 delta)
{
	assert(self != NULL);
	if (delta < 0)
	{
		for (u32 idx = self->curx + self->freeSpaceLen; delta < 0 && self->curx > 0; ++delta)
		{
			--idx;
			--self->curx;
			self->line[idx] = self->line[self->curx];
		}
	}
	else
	{
		for (u32 total = self->lineEndx - self->freeSpaceLen, idx = self->curx + self->freeSpaceLen; delta > 0 && self->curx < total; --delta)
		{
			self->line[self->curx] = self->line[idx];
			++idx;
			++self->curx;
		}
	}
}
void fLine_moveCursorAbs(fLine_t * restrict self, u32 curx)
{
	assert(self != NULL);
	if (curx < self->curx)
	{
		for (u32 idx = self->curx + self->freeSpaceLen; curx != self->curx && self->curx > 0;)
		{
			--idx;
			--self->curx;
			self->line[idx] = self->line[self->curx];
		}
	}
	else
	{
		for (u32 total = self->lineEndx - self->freeSpaceLen, idx = self->curx + self->freeSpaceLen; curx != self->curx && self->curx < total;)
		{
			self->line[self->curx] = self->line[idx];
			++idx;
			++self->curx;
		}
	}
}
void fLine_moveCursorVert(fLine_t ** restrict self, i32 delta)
{
	assert(self != NULL);
	assert(*self != NULL);

	fLine_t * node = *self;
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
void fLine_calcVirtCursor(fLine_t * restrict self, u32 tabWidth)
{
	assert(self != NULL);
	assert(tabWidth > 0);
	self->virtcurx = 0;
	for (u32 i = 0; i < self->curx; ++i)
	{
		self->virtcurx += (self->line[i] == L'\t') ? tabWidth - (self->virtcurx % tabWidth) : 1;
	}
}
u32 fLine_calcCursor(const fLine_t * restrict self, u32 virtcur, u32 tabWidth)
{
	assert(self != NULL);
	assert(tabWidth > 0);

	u32 realcurx = 0;
	for (u32 i = 0, curx = 0; (curx < virtcur) && (i < self->lineEndx);)
	{
		if ((i == self->curx) && (self->freeSpaceLen > 0))
		{
			i += self->freeSpaceLen;
			continue;
		}
		curx += (self->line[i] == L'\t') ? tabWidth - (curx % tabWidth) : 1;
		++realcurx;
		++i;
	}

	return realcurx;
}

void fLine_swap(fLine_t * restrict node1, fLine_t * restrict node2)
{
	assert(node1 != NULL);
	assert(node2 != NULL);

	fLine_t temp = *node1;
	node1->line         = node2->line;
	node1->lineEndx     = node2->lineEndx;
	node1->curx         = node2->curx;
	node1->freeSpaceLen = node2->freeSpaceLen;
	node1->virtcurx     = node2->virtcurx;
	node1->syntax       = node2->syntax;

	node2->line         = temp.line;
	node2->lineEndx     = temp.lineEndx;
	node2->curx         = temp.curx;
	node2->freeSpaceLen = temp.freeSpaceLen;
	node2->virtcurx     = temp.virtcurx;
	node2->syntax       = temp.syntax;
}

void fLine_updateLineNumbers(fLine_t * restrict startnode, u32 startLno, uint8_t * restrict noLen)
{
	fLine_t * prevnode = NULL;
	while (startnode != NULL)
	{
		startnode->lineNumber = startLno;
		++startLno;

		prevnode  = startnode;
		startnode = startnode->nextNode;
	}
	if ((prevnode != NULL) && (prevnode->lineNumber > 0))
	{
		*noLen = (uint8_t)log10((f64)prevnode->lineNumber) + 1;
	}
	else
	{
		*noLen = 1;
	}
}

bool fLine_updateSyntax(
	fLine_t * restrict node,
	enum fSyntax fs,
	const WORD * colors
)
{
	assert(node != NULL);

	switch (fs)
	{
	case fstxC:
		return fStx_parseCLike(node, colors, &fStx_checkCToken, fstxC);
	case fstxCPP:
		return fStx_parseCLike(node, colors, &fStx_checkCPPToken, fstxCPP);
	case fstxMD:
		return fStx_parseMd(node, colors);
	case fstxPY:
		return fStx_parsePy(node, colors);
	case fstxJS:
		return fStx_parseCLike(node, colors, &fStx_checkJSToken, fstxJS);
	case fstxJSON:
		return fStx_parseJSON(node, colors);
	case fstxCSS:
		return fStx_parseCSS(node, colors);
	case fstxXML:
	case fstxSVG:
		return fStx_parseXML(node, colors);
	case fstxHTML:
		return fStx_parseXML(node, colors);
	case fstxRust:
		return fStx_parseCLike(node, colors, &fStx_checkRustToken, fstxRust);
	case fstxGo:
		return fStx_parseCLike(node, colors, &fStx_checkGoToken, fstxGo);

	default:
		return fStx_parseNone(node, colors);
	}
}

void fLine_destroy(fLine_t * restrict self)
{
	assert(self != NULL);
	
	free(self->line);
	self->line = NULL;

	free(self->syntax);
	self->syntax = NULL;
}
void fLine_free(fLine_t * restrict self)
{
	assert(self != NULL);
	fLine_destroy(self);
	free(self);
}

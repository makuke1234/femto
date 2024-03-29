#include "fLine.h"
#include "fSyntax.h"
#include "fSettings.h"
#include "fFile.h"
#include "femto.h"

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
		.userValue    = { 0 }
	};
}
fLine_t * fLine_create(
	fLine_t * restrict curnode,
	fLine_t * restrict nextnode,
	bool tabsToSpaces,
	u8 tabWidth,
	bool autoIndent,
	u8 * restrict noLen
)
{
	assert(tabWidth > 0);
	assert(noLen    != NULL);

	fLine_t * restrict node = malloc(sizeof(fLine_t));
	if (node == NULL)
	{
		return NULL;
	}

	// Select space when tabs must be converted to spaces
	const wchar tch = L"\t "[tabsToSpaces];

	usize space = 0;
	if (curnode != NULL)
	{
		if (autoIndent)
		{
			for (usize i = 0; i < curnode->lineEndx;)
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
				space /= (usize)tabWidth;
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
			for (usize i = 0; i < space; ++i)
			{
				node->line[i] = tch;
			}
		}
		// Copy contents after cursor to this line
		else
		{
			const usize contStart = curnode->curx + curnode->freeSpaceLen, contLen = curnode->lineEndx - contStart;
			node->lineEndx = contLen + FEMTO_LNODE_DEFAULT_FREE + space;
			node->line = malloc(sizeof(wchar) * node->lineEndx);
			if (node->line == NULL)
			{
				free(node);
				return NULL;
			}
			for (usize i = 0; i < space; ++i)
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
	node->userValue.val = 0;

	return node;
}

fLine_t * fLine_createText(
	fLine_t * restrict curnode,
	fLine_t * restrict nextnode,
	const wchar * restrict lineText,
	isize mText,
	u8 * restrict noLen
)
{
	assert(lineText != NULL);
	assert(noLen    != NULL);

	const usize maxText = (mText == -1) ? wcslen(lineText) : (usize)mText;

	fLine_t * restrict node = malloc(sizeof(fLine_t));
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
	node->userValue.val = 0;

	return node;
}

bool fLine_getText(const fLine_t * restrict self, wchar ** restrict text, usize * restrict tarrsz)
{
	assert(self != NULL);
	assert(text != NULL);

	const usize totalLen = self->lineEndx - self->freeSpaceLen + 1;

	if ((tarrsz != NULL) && (*tarrsz < totalLen))
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

	wchar * restrict t = *text;
	for (usize i = 0; i < self->lineEndx;)
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
void fLine_getTextLim(const fLine_t * restrict self, wchar * restrict text, usize maxLen)
{
	assert(self   != NULL);
	assert(text   != NULL);
	assert(maxLen > 0);

	usize len = 0;
	for (usize i = 0; (i < self->lineEndx) && (len < (maxLen - 1));)
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
	const usize totalLen = self->lineEndx - self->freeSpaceLen;
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

bool fLine_addChar(fLine_t * restrict self, wchar ch, u8 tabWidth)
{
	assert(self     != NULL);
	assert(ch       != L'\0');
	assert(tabWidth > 0);

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
bool fLine_checkAt(const fLine_t * restrict node, isize maxdelta, const wchar * restrict string, usize maxString)
{
	assert(string != NULL);
	
	if ((node == NULL) || (maxString == 0))
	{
		return false;
	}

	isize idx = (isize)node->curx + maxdelta, i = 0;
	idx += (maxdelta > 0) ? (isize)node->freeSpaceLen : 0;
	const isize m = (isize)maxString;
	if (idx < 0)
	{
		return false;
	}
	for (; (idx < (isize)node->lineEndx) && (i < m) && ((*string) != L'\0');)
	{
		if ((idx == (isize)node->curx) && (node->freeSpaceLen > 0))
		{
			idx += (isize)node->freeSpaceLen;
			continue;
		}
		else if (node->line[idx] != (*string))
		{
			return false;
		}

		++string;
		++i;
		++idx;
	}
	if ((i < m) && ((*string) != L'\0'))
	{
		return false;
	}

	return true;
}
usize fLine_find(const fLine_t * restrict node, usize startIdx, const wchar * restrict string, usize maxString)
{
	assert(node      != NULL);
	assert(string    != NULL);
	assert(maxString > 0);

	// Clamp startIdx
	const usize cur2 = node->curx + node->freeSpaceLen;
	startIdx = ((startIdx >= node->curx) && (startIdx < cur2)) ? cur2 : startIdx;

	for (usize i = startIdx; i < node->lineEndx;)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i = cur2;
			continue;
		}
		else if (node->line[i] == string[0])
		{
			const wchar * str = string;
			usize k = 0;
			for (usize j = i; (j < node->lineEndx) && (k < maxString);)
			{
				if ((j == node->curx) && (node->freeSpaceLen > 0))
				{
					j = cur2;
					continue;
				}
				else if ((*str == L'\0') || (node->line[j] != *str))
				{
					break;
				}
				++str;
				++k;
				++j;
			}

			if ((k == maxString) || (*str == L'\0'))
			{
				return i;
			}
		}

		++i;
	}

	return UINT32_MAX;
}
usize fLine_rfind(const fLine_t * restrict node, usize startIdx, const wchar * restrict string, usize maxString)
{
	assert(node      != NULL);
	assert(string    != NULL);
	assert(maxString > 0);

	// Clamp startIdx
	const usize cur2 = node->curx + node->freeSpaceLen;
	startIdx = ((startIdx >= node->curx) && (startIdx < cur2)) ? cur2 : startIdx;

	for (usize i = startIdx; 1;)
	{
		if ((i == (cur2 - 1)) && (node->freeSpaceLen > 0))
		{
			i -= node->freeSpaceLen;
			continue;
		}
		else if (node->line[i] == string[0])
		{
			const wchar * str = string;
			usize k = 0;
			for (usize j = i; (j < node->lineEndx) && (k < maxString);)
			{
				if ((j == node->curx) && (node->freeSpaceLen > 0))
				{
					j = cur2;
					continue;
				}
				else if ((*str == L'\0') || (node->line[j] != *str))
				{
					break;
				}
				++str;
				++k;
				++j;
			}

			if ((k == maxString) || (*str == L'\0'))
			{
				return i;
			}
		}
		if (i == 0)
		{
			break;
		}

		--i;
	}

	return UINT32_MAX;
}

bool fLine_mergeNext(fLine_t * restrict self, fLine_t ** restrict ppcury, u8 * restrict noLen)
{
	assert(self   != NULL);
	assert(ppcury != NULL);
	assert(noLen  != NULL);

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
	fLine_moveCursor(self, (isize)self->lineEndx);
	fLine_moveCursor(n,    (isize)n->lineEndx);

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

void fLine_moveCursor(fLine_t * restrict self, isize delta)
{
	assert(self != NULL);

	if (delta < 0)
	{
		for (usize idx = self->curx + self->freeSpaceLen; delta < 0 && self->curx > 0; ++delta)
		{
			--idx;
			--self->curx;
			self->line[idx] = self->line[self->curx];
		}
	}
	else if (delta > 0)
	{
		for (usize total = self->lineEndx - self->freeSpaceLen, idx = self->curx + self->freeSpaceLen; delta > 0 && self->curx < total; --delta)
		{
			self->line[self->curx] = self->line[idx];
			++idx;
			++self->curx;
		}
	}
}
void fLine_moveCursorAbs(fLine_t * restrict self, usize curx)
{
	assert(self != NULL);

	if (curx < self->curx)
	{
		for (usize idx = self->curx + self->freeSpaceLen; curx != self->curx && self->curx > 0;)
		{
			--idx;
			--self->curx;
			self->line[idx] = self->line[self->curx];
		}
	}
	else
	{
		for (usize total = self->lineEndx - self->freeSpaceLen, idx = self->curx + self->freeSpaceLen; curx != self->curx && self->curx < total;)
		{
			self->line[self->curx] = self->line[idx];
			++idx;
			++self->curx;
		}
	}
}
void fLine_moveCursorVert(fLine_t ** restrict self, isize delta)
{
	assert(self  != NULL);
	assert(*self != NULL);

	fLine_t * restrict node = *self;
	if (delta < 0)
	{
		for (; delta != 0 && node->prevNode != NULL; ++delta)
		{
			node = node->prevNode;
		}
	}
	else if (delta > 0)
	{
		for (; delta != 0 && node->nextNode != NULL; --delta)
		{
			node = node->nextNode;
		}
	}
	*self = node;
}
void fLine_calcVirtCursor(fLine_t * restrict self, usize tabWidth)
{
	assert(self     != NULL);
	assert(tabWidth > 0);

	self->virtcurx = 0;
	for (usize i = 0; i < self->curx; ++i)
	{
		self->virtcurx += (self->line[i] == L'\t') ? tabWidth - (self->virtcurx % tabWidth) : 1;
	}
}
usize fLine_calcCursor(const fLine_t * restrict self, usize virtcur, u8 tabWidth)
{
	assert(self     != NULL);
	assert(tabWidth > 0);

	usize realcurx = 0;
	for (usize i = 0, curx = 0; (curx < virtcur) && (i < self->lineEndx);)
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
	node1->userValue    = node2->userValue;

	node2->line         = temp.line;
	node2->lineEndx     = temp.lineEndx;
	node2->curx         = temp.curx;
	node2->freeSpaceLen = temp.freeSpaceLen;
	node2->virtcurx     = temp.virtcurx;
	node2->syntax       = temp.syntax;
	node2->userValue    = temp.userValue;
}

void fLine_updateLineNumbers(fLine_t * restrict startnode, usize startLno, u8 * restrict noLen)
{
	assert(noLen != NULL);
	
	const fLine_t * restrict prevnode = NULL;
	while (startnode != NULL)
	{
		startnode->lineNumber = startLno;
		++startLno;

		prevnode  = startnode;
		startnode = startnode->nextNode;
	}
	if ((prevnode != NULL) && (prevnode->lineNumber > 0))
	{
		*noLen = (u8)log10((f64)prevnode->lineNumber) + 1;
	}
	else
	{
		*noLen = 1;
	}
}

bool fLine_updateSyntax(
	fLine_t * restrict node, fStx_e fs, const WORD * colors,
	const wchar * restrict searchTerm, fSearch_e searchOpts, const struct fFileHighLight * restrict hl,
	usize curLineNum, u8 tabWidth
)
{
	assert(node       != NULL);
	assert(colors     != NULL);
	assert(hl         != NULL);
	assert(curLineNum > 0);
	assert(tabWidth   > 0);

	bool ret;

	switch (fs)
	{
	case fstxC:
		ret = fStx_parseCLike(node, colors, &fStx_checkCToken, fstxC);
		break;
	case fstxCPP:
		ret = fStx_parseCLike(node, colors, &fStx_checkCPPToken, fstxCPP);
		break;
	case fstxMD:
		ret = fStx_parseMd(node, colors);
		break;
	case fstxPY:
		ret = fStx_parsePy(node, colors);
		break;
	case fstxJS:
		ret = fStx_parseCLike(node, colors, &fStx_checkJSToken, fstxJS);
		break;
	case fstxJSON:
		ret = fStx_parseJSON(node, colors);
		break;
	case fstxCSS:
		ret = fStx_parseCSS(node, colors);
		break;
	case fstxXML:
	case fstxSVG:
		ret = fStx_parseXML(node, colors);
		break;
	case fstxHTML:
		ret = fStx_parseXML(node, colors);
		break;
	case fstxRust:
		ret = fStx_parseCLike(node, colors, &fStx_checkRustToken, fstxRust);
		break;
	case fstxGo:
		ret = fStx_parseCLike(node, colors, &fStx_checkGoToken, fstxGo);
		break;

	default:
		ret = fStx_parseNone(node, colors);
	}
	
	if (!ret)
	{
		return false;
	}

	node->userValue.bits.b8 = false;
	// Find term
	if (searchTerm != NULL)
	{
		const usize termLen = wcslen(searchTerm);
		usize idx = 0, firstidx = 0;
		bool first = true;
		while (1)
		{
			idx = fLine_find(node, idx, searchTerm, termLen);
			if (idx == UINT32_MAX)
			{
				break;
			}
			
			switch (searchOpts)
			{
			case fsrchFIRST:
				firstidx = first ? idx : firstidx;
				first = false;
				break;
			case fsrchLAST:
				firstidx = idx;
				first = false;
				break;
			case fsrchPREV:
				firstidx = (idx < node->curx) ? idx : firstidx;
				first = false;
				break;
			case fsrchNEXT:
				if (first && (idx > (node->curx + node->freeSpaceLen)))
				{
					firstidx = idx;
					first = false;
				}
				break;
			}
			// Found
			node->userValue.bits.b8 = true;

			// Highlighting index
			for (usize hidx = (idx > node->curx) ? (idx - node->freeSpaceLen) : idx, stop = hidx + termLen; hidx < stop; ++hidx)
			{
				node->syntax[hidx] = colors[tcSEARCH_RESULT];
			}

			idx += ((idx < node->curx) && ((idx + termLen) >= node->curx) && (node->freeSpaceLen > 0)) ? termLen + node->freeSpaceLen : termLen;
		}
		// Move cursor to term if found
		if (!first && node->userValue.bits.b7)
		{
			fLine_moveCursorAbs(node, (firstidx > node->curx) ? (firstidx - node->freeSpaceLen) : firstidx);
			fLine_calcVirtCursor(node, tabWidth);
		}
		node->userValue.bits.b7 = false;
	}
	if (hl->beg != NULL)
	{
		// Be picky about what to highlight
		usize begCur, endCur;
		if (hl->beg == node)
		{
			// We are on the current line, whole selection is limited to 1 line
			if (curLineNum == node->lineNumber)
			{
				// Only highlight what's between cursors
				begCur = hl->backwards ? node->curx : hl->begx;
				endCur = hl->backwards ? hl->begx   : node->curx;
			}
			else
			{
				// Highlighting everything from/to cursor
				begCur = hl->backwards ? 0        : hl->begx;
				endCur = hl->backwards ? hl->begx : node->lineEndx - node->freeSpaceLen;
			}

			for (; begCur < endCur; ++begCur)
			{
				node->syntax[begCur] = colors[tcHIGHLIGHT];
			}
		}
		// Highlight whole line, go haywire d: ;P :b B-)
		else if (node->lineNumber == clampdc_usize(node->lineNumber, hl->beg->lineNumber, curLineNum))
		{
			
			// Detect "edge" line
			if (node->lineNumber == hl->beg->lineNumber)
			{
				begCur = hl->backwards ? 0        : hl->begx;
				endCur = hl->backwards ? hl->begx : node->lineEndx - node->freeSpaceLen;
			}
			else if (node->lineNumber == curLineNum)
			{
				begCur = hl->backwards ? node->curx                          : 0;
				endCur = hl->backwards ? node->lineEndx - node->freeSpaceLen : node->curx;
			}
			else
			{
				begCur = 0;
				endCur = node->lineEndx - node->freeSpaceLen;
			}
			
			for (; begCur < endCur; ++begCur)
			{
				node->syntax[begCur] = colors[tcHIGHLIGHT];
			}
		}
		// Otherwise highlight nothing
	}

	return true;
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

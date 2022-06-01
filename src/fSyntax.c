#include "fSyntax.h"
#include "fLine.h"
#include "fSettings.h"
#include "fFile.h"
#include "fStatHashmap.h"

#include "keywords/fKeywordsC.h"
#include "keywords/fKeywordsCPP.h"
#include "keywords/fKeywordsPy.h"
#include "keywords/fKeywordsJS.h"
#include "keywords/fKeywordsRust.h"
#include "keywords/fKeywordsGo.h"

fStx_e fStx_detect(const wchar * restrict fileName)
{
	assert(fileName != NULL);
	
	fStx_e syntax = fstxNONE;
	
	const wchar * restrict end = fileName + wcslen(fileName);
	if (fileName != end)
	{
		const wchar * restrict dot = end - 1;
		for (u8 i = 0; (dot != fileName) && (i < (MAX_SUFFIX - 1)); --dot, ++i)
		{
			if (*dot == L'.')
			{
				break;
			}
		}

		if (*dot == L'.')
		{
			++dot;

			wchar suffix[MAX_SUFFIX];
			++end;
			for (wchar * suf = suffix; dot != end; ++dot, ++suf)
			{
				*suf = (wchar)towlower(*dot);
			}

			if ((wcscmp(suffix, L"c") == 0) || (wcscmp(suffix, L"h") == 0))
			{
				syntax = fstxC;
			}
			else if ((wcscmp(suffix, L"C") == 0) || (wcscmp(suffix, L"cc") == 0) ||
				(wcscmp(suffix, L"cpp") == 0) || (wcscmp(suffix, L"cxx") == 0) || (wcscmp(suffix, L"c++") == 0) ||
				(wcscmp(suffix, L"H") == 0) || (wcscmp(suffix, L"hh") == 0) ||
				(wcscmp(suffix, L"hpp") == 0) || (wcscmp(suffix, L"hxx") == 0) || (wcscmp(suffix, L"h++") == 0))
			{
				syntax = fstxCPP;
			}
			else if ((wcscmp(suffix, L"md") == 0) || (wcscmp(suffix, L"markdown") == 0))
			{
				syntax = fstxMD;
			}
			else if (wcscmp(suffix, L"py") == 0)
			{
				syntax = fstxPY;
			}
			else if (wcscmp(suffix, L"js") == 0)
			{
				syntax = fstxJS;
			}
			else if (wcscmp(suffix, L"json") == 0)
			{
				syntax = fstxJSON;
			}
			else if (wcscmp(suffix, L"css") == 0)
			{
				syntax = fstxCSS;
			}
			else if (wcscmp(suffix, L"xml") == 0)
			{
				syntax = fstxXML;
			}
			else if ((wcscmp(suffix, L"html") == 0) || (wcscmp(suffix, L"htm") == 0))
			{
				syntax = fstxHTML;
			}
			else if (wcscmp(suffix, L"svg") == 0)
			{
				syntax = fstxSVG;
			}
			else if (wcscmp(suffix, L"rs") == 0)
			{
				syntax = fstxRust;
			}
			else if (wcscmp(suffix, L"go") == 0)
			{
				syntax = fstxGo;
			}
		}
	}

	return syntax;
}

const char * fStx_name(fStx_e fs)
{
	static const char * syntaxes[fstxSIZE] = {
		[fstxNONE] = "None",
		[fstxC]    = "C",
		[fstxCPP]  = "C++",
		[fstxMD]   = "Markdown",
		[fstxPY]   = "Python",
		[fstxJS]   = "ECMAScript (JavaScript)",
		[fstxJSON] = "JavaScript Object Notation (JSON)",
		[fstxCSS]  = "Cascading Style Sheets (CSS)",
		[fstxXML]  = "eXtensible Markup Language (XML)",
		[fstxHTML] = "HyperText Markup Language (HTML)",
		[fstxSVG]  = "Scalable Vector Graphics (SVG)",
		[fstxRust] = "Rust",
		[fstxGo]   = "Go (Golang)"
	};

	assert(fs < fstxSIZE);
	return syntaxes[fs];
}
bool fStx_autoAlloc(fLine_t * restrict node)
{
	assert(node != NULL);

	vptr mem = realloc(node->syntax, sizeof(WORD) * node->lineEndx);
	if (mem == NULL)
	{
		free(node->syntax);
		node->syntax = NULL;
		return false;
	}
	node->syntax = mem;

	return true;
}

void fStx_checkGenericToken(
	fLine_t * restrict node, u32 start, u32 lasti,
	WORD kwCol,
	const fStatHash_t * restrict map
)
{
	assert(node != NULL);
	
	if ((lasti - start) < 1)
	{
		return;
	}

	wchar kwBuf[MAX_KWBUF];
	// Null-temrinate for safety
	kwBuf[MAX_KWBUF - 1] = L'\0';

	u32 filled = 0;
	for (i32 i = MAX_KWBUF - 2, j = (i32)lasti, starti = (i32)start; (i >= 0) && (j >= 0) && (j >= starti);)
	{
		if (((j - (i32)node->freeSpaceLen) == ((i32)node->curx - 1)) && (node->freeSpaceLen > 0))
		{
			j -= (i32)node->freeSpaceLen;
			continue;
		}
		kwBuf[i] = node->line[j];
		--i;
		--j;
		++filled;
	}

	for (u32 i = 0; i < (MAX_C_TOKEN_WORD - 1); ++i)
	{
		const u32 n = MAX_C_TOKEN_WORD - i;
		if ((n > filled) || ((filled - n) > 1))
		{
			continue;
		}
		else if (fStatHash_get(map, kwBuf + (MAX_KWBUF - 1) - n))
		{
			i32 l = (lasti > node->curx) ? (i32)(lasti - node->freeSpaceLen) : (i32)lasti;
			for (u32 k = 0; k < n; --l, ++k)
			{
				node->syntax[l] = kwCol;
			}
			return;
		}
	}
}
void fStx_checkCToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_C_TOKEN_MEM];
	static fStatHash_t map = { 0 };
	
	fStatHash_initData(&map, memory, sizeof(usize) * MAX_C_TOKEN_MEM, s_keyWordsC, ARRAYSIZE(s_keyWordsC));
	fStx_checkGenericToken(node, start, lasti, kwCol, &map);
}
void fStx_checkCPPToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_CPP_TOKEN_MEM];
	static fStatHash_t map = { 0 };

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_CPP_TOKEN_MEM, s_keyWordsCPP, ARRAYSIZE(s_keyWordsCPP));
	fStx_checkGenericToken(node, start, lasti, kwCol, &map);
}
void fStx_checkPyToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_PY_TOKEN_MEM];
	static fStatHash_t map = { 0 };

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_PY_TOKEN_MEM, s_keyWordsPy, ARRAYSIZE(s_keyWordsPy));
	fStx_checkGenericToken(node, start, lasti, kwCol, &map);
}
void fStx_checkJSToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_JS_TOKEN_MEM];
	static fStatHash_t map = { 0 };

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_JS_TOKEN_MEM, s_keyWordsJS, ARRAYSIZE(s_keyWordsJS));
	fStx_checkGenericToken(node, start, lasti, kwCol, &map);
}
void fStx_checkRustToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_RUST_TOKEN_MEM];
	static fStatHash_t map = { 0 };
	
	fStatHash_initData(&map, memory, sizeof(usize) * MAX_RUST_TOKEN_MEM, s_keyWordsRust, ARRAYSIZE(s_keyWordsRust));
	fStx_checkGenericToken(node, start, lasti, kwCol, &map);
}
void fStx_checkGoToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_GO_TOKEN_MEM];
	static fStatHash_t map = { 0 };
	
	fStatHash_initData(&map, memory, sizeof(usize) * MAX_GO_TOKEN_MEM, s_keyWordsGo, ARRAYSIZE(s_keyWordsGo));
	fStx_checkGenericToken(node, start, lasti, kwCol, &map);
}

bool fStx_parseNone(fLine_t * restrict node, const WORD * restrict colors)
{
	if (!fStx_autoAlloc(node))
	{
		return false;
	}

	for (u32 i = 0, j = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}

		node->syntax[j] = colors[tcTEXT];
	}

	return true;
}
bool fStx_parseCLike(
	fLine_t * restrict node,
	const WORD * restrict colors,
	fStx_tokeniserFunc_t func,
	fStx_e lang
)
{
	if (!fStx_autoAlloc(node))
	{
		return false;
	}

	// Calculate feature allowances
	const bool bAllowPreproc = (lang == fstxC) | (lang == fstxCPP);

	u32 tokenStart = 0;
	bool quoteMode = false, littleQuote = false, skip = false, letter = false,
		isZero = false, hex = false, octal = false, comment = false, blockComment = false, preproc = false;
	
	blockComment = (node->prevNode != NULL) ? node->prevNode->userValue.bits.b1 : false;

	u32 previ = 0;
	for (u32 i = 0, j = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}
		node->syntax[j] = colors[tcTEXT];


		const wchar ch = node->line[i];
		
		if (blockComment)
		{
			if ((ch == L'/') && (j > 0) && (node->line[previ] == L'*'))
			{
				blockComment = false;
				tokenStart = i;
			}
			node->syntax[j] = colors[tcCOMMENT_BLOCK];
			previ = i;
			continue;
		}
		else if (comment)
		{
			node->syntax[j] = colors[tcCOMMENT_LINE];
			previ = i;
			continue;
		}
		else if (quoteMode)
		{
			node->syntax[j] = colors[littleQuote ? tcCHARACTER : tcSTRING];
			if (skip)
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = false;
			}
			else if (ch == L'\\')
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = true;
			}
			else if ((!littleQuote && (ch == L'"')) || (littleQuote && (ch == L'\'')))
			{
				quoteMode = false;
				littleQuote = false;
				tokenStart = i;
			}
			previ = i;
			continue;
		}
		else if ((ch == L'*') && (j > 0) && (node->line[previ] == L'/'))
		{
			blockComment = true;
			node->syntax[j-1] = colors[tcCOMMENT_BLOCK];
			node->syntax[j]   = colors[tcCOMMENT_BLOCK];
			previ = i;
			continue;
		}
		else if ((ch == '/') && (j > 0) && (node->line[previ] == L'/'))
		{
			comment = true;
			node->syntax[j-1] = colors[tcCOMMENT_LINE];
			node->syntax[j]   = colors[tcCOMMENT_LINE];
			previ = i;
			continue;
		}
		else if (isZero)
		{
			isZero = false;
			if (ch == L'x')
			{
				letter = false;
				hex = true;
				node->syntax[j] = colors[tcHEX];
				previ = i;
				continue;
			}
			if ((ch >= L'0') && (ch <= '7'))
			{
				octal = true;
			}
		}
		else if (preproc)
		{
			node->syntax[j] = colors[tcPREPROC];
		}
		
		switch (ch)
		{
			case L'(':
			case L')':
			case L'{':
			case L'}':
			case L'[':
			case L']':
			case L'.':
			case L',':
			case L'*':
			case L'+':
			case L'-':
			case L'/':
			case L'&':
			case L'|':
			case L'<':
			case L'>':
			case L'=':
			case L';':
			case L':':
				node->syntax[j] = colors[tcPUNCTUATION];
				/* fall through */
			case L' ':
			case L'\t':
				letter = false;
				func(node, tokenStart, previ, colors[tcKEYWORD]);
				tokenStart = i;
				break;
			case L'\'':
				littleQuote = true;
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcCHARACTER_QUOTE];
				break;
			case L'"':
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcSTRING_QUOTE];
				break;
			case L'#':
				if (bAllowPreproc)
				{
					preproc = true;
					break;
				}
				/* fall through */
			default:
				if (hex)
				{
					const wchar lch = (wchar)towlower(ch);
					if (!letter && (((ch >= L'0') && (ch <= L'9')) || ((lch >= L'a') && (lch <= L'f'))))
					{
						node->syntax[j] = colors[tcNUMBER];
					}
					else
					{
						hex = false;
						tokenStart = i;
					}
				}
				else if (isZero || octal)
				{
					isZero = false;
					if (!letter && ((ch >= L'0') && (ch <= '7')))
					{
						octal = true;
						node->syntax[j] = colors[tcOCT];
					}
					else
					{
						octal = false;
						tokenStart = i;
					}
				}
				else if ((ch >= L'0') && (ch <= L'9'))
				{
					if (!letter)
					{
						isZero = (ch == L'0');
						node->syntax[j] = colors[tcNUMBER];
						tokenStart = i;
					}
				}
				else
				{
					const wchar lch = (wchar)towlower(ch);
					letter = ((lch >= L'a') && (lch <= L'z')) || (ch == L'_');
					if (!letter)
					{
						tokenStart = i;
					}
				}
		}

		previ = i;
	}

	if (!blockComment && !comment && !preproc && !quoteMode)
	{
		func(node, tokenStart, previ, colors[tcKEYWORD]);
	}

	node->userValue.bits.b1 = blockComment;

	return true;
}
bool fStx_parseMd(fLine_t * restrict node, const WORD * restrict colors)
{
	if (!fStx_autoAlloc(node))
	{
		return false;
	}
	
	/*
		Markdown logic:
		
		Find '#' at the beginning of the line, there can be less than tabWidth worth of spaces
		Count number of spaces at beginning of line, if more than tabWidth, highlight
		If less than tabWidth search dash '-'
		
		Patterns:
		![asd]
		[asd]
		...[asd](text)
		*asd* & _asd_
		**asd** & __asd__
		~~sdd~~
		
	*/
	
	u32 firstChIdx = 0;
	bool done = false, headingMode = false, valueMode = false, bracketMode = false,
		extraBracketMode = false, italicsMode = false, boldMode = false,
		containsStar = false, strikeMode = false, parenMode1 = false,
		parenMode2 = false, enable = false, coneMode = false, codeMode = false;
	
	for (u32 i = 0, j = 0, previ = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}
		node->syntax[j] = colors[tcTEXT];

		const wchar ch = node->line[i];
		
		if (valueMode)
		{
			node->syntax[j] = colors[tcMD_VALUE];
		}
		else if (codeMode)
		{
			if (ch == L'`')
			{
				codeMode = false;
			}
			node->syntax[j] = colors[tcMD_VALUE];
		}
		else if (coneMode)
		{
			node->syntax[j] = colors[tcMD_CONETEXT];
		}
		else if (strikeMode)
		{
			if ((ch == L'~') && (node->line[previ] == L'~'))
			{
				strikeMode = false;
				node->syntax[j-1] = colors[tcPUNCTUATION];
				node->syntax[j]   = colors[tcPUNCTUATION];
			}
			else
			{
				node->syntax[j] = colors[tcMD_STRIKE];
			}
		}
		else if (italicsMode)
		{
			if (containsStar && ( ( (ch == L'*') && (node->line[previ] == L'*') ) || ( (ch == L'_') && (node->line[previ] == L'_') ) ) )
			{
				boldMode = true;
				italicsMode = false;
				node->syntax[j]   = colors[tcMD_BOLD];
				node->syntax[j-1] = colors[tcMD_BOLD];
			}
			else if ((ch == L'*') || (ch == L'_'))
			{
				italicsMode = false;
				node->syntax[j] = colors[tcMD_ITALIC];
			}
			else
			{
				node->syntax[j] = colors[tcMD_ITALIC];
			}

			containsStar = false;
		}
		else if (boldMode)
		{
			if ( ( (ch == L'*') && (node->line[previ] == L'*') ) || ( (ch == L'_') && (node->line[previ] == L'_') ) )
			{
				boldMode = false;
				node->syntax[j] = colors[tcMD_BOLD];
			}
			else
			{
				node->syntax[j] = colors[tcMD_BOLD];
			}
		}
		else if (bracketMode)
		{
			if (ch == L']')
			{
				bracketMode = false;
				extraBracketMode = false;
				parenMode1 = true;
				node->syntax[j] = colors[tcPUNCTUATION];
			}
			else
			{
				node->syntax[j] = colors[extraBracketMode ? tcMD_BRACKETPIC : tcMD_BRACKET];
			}
		}
		else if (parenMode1)
		{
			parenMode1 = false;
			if (ch == L'(')
			{
				parenMode2 = true;
				node->syntax[j] = colors[tcPUNCTUATION];
			}
		}
		else if (parenMode2)
		{
			if (ch == L')')
			{
				parenMode2 = false;
				node->syntax[j] = colors[tcPUNCTUATION];
			}
			else
			{
				node->syntax[j] = colors[tcMD_LINK];
			}
		}
		else if (!done)
		{
			if (ch == L'#')
			{
				headingMode = true;
				node->syntax[j] = colors[tcMD_HEADING];
			}
			else if ((ch == L'-') || (ch == L'>'))
			{
				// Dash coloring
				node->syntax[j] = colors[tcPUNCTUATION];
				coneMode = (ch == L'>');
			}

			if ((ch != L' ') && (ch != L'\t'))
			{
				done = true;
				enable = true;
				if (firstChIdx >= 4)
				{
					valueMode = true;
					node->syntax[j] = colors[tcMD_VALUE];
				}
			}
			else if (ch == L' ')
			{
				firstChIdx += 1;
			}
			else if (ch == L'\t')
			{
				firstChIdx += 4;
			}
		}
		else
		{
			enable = true;
		}

		if (enable)
		{
			enable = false;

			if (headingMode)
			{
				node->syntax[j] = colors[tcMD_HEADING];
			}
			if (ch == L'[')
			{
				bracketMode = true;
				if ((j > 0) && (node->line[previ] == L'!'))
				{
					node->syntax[j-1] = colors[tcPUNCTUATION];
					extraBracketMode = true;
				}
				node->syntax[j] = colors[tcPUNCTUATION];
			}
			else if ((ch == L'*') || (ch == L'_'))
			{
				containsStar = true;
				italicsMode = true;
				node->syntax[j] = colors[tcMD_ITALIC];
			}
			else if ((ch == L'~') && (j > 0) && (node->line[previ] == L'~'))
			{
				strikeMode = true;
				node->syntax[j-1] = colors[tcPUNCTUATION];
				node->syntax[j]   = colors[tcPUNCTUATION];
			}
			else if (ch == L'`')
			{
				codeMode = true;
				node->syntax[j] = colors[tcMD_VALUE];
			}
		}

		previ = i;
	}

	return true;
}

bool fStx_parsePy(fLine_t * restrict node, const WORD * restrict colors)
{
	if (!fStx_autoAlloc(node))
	{
		return false;
	}

	u32 tokenStart = 0;
	bool quoteMode = false, littleQuote = false, skip = false, letter = false,
		isZero = false, hex = false, octal = false, comment = false, blockComment = false,
		preComment = false, firstQuote = false;
	
	littleQuote  = (node->prevNode != NULL) ? node->prevNode->userValue.bits.b2 : false;
	blockComment = (node->prevNode != NULL) ? node->prevNode->userValue.bits.b1 : false;

	u32 previ = 0, prevprevi = UINT32_MAX;
	for (u32 i = 0, j = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}
		node->syntax[j] = colors[tcTEXT];


		const wchar ch = node->line[i];
		
		if (blockComment)
		{
			if ((j > 1) && (prevprevi != UINT32_MAX) && ((!littleQuote && (ch == L'"') && (node->line[previ] == L'"') && (node->line[prevprevi] == L'"')) ||
				(littleQuote && (ch == L'\'') && (node->line[previ] == L'\'') && (node->line[prevprevi] == L'\''))) )
			{
				blockComment = false;
				littleQuote = false;
				tokenStart = i;
			}
			node->syntax[j] = colors[tcCOMMENT_BLOCK];
			prevprevi = previ;
			previ = i;
			continue;
		}
		else if (comment)
		{
			node->syntax[j] = colors[tcCOMMENT_LINE];
			previ = i;
			continue;
		}
		else if (quoteMode)
		{
			node->syntax[j] = colors[littleQuote ? tcCHARACTER : tcSTRING];
			if (skip)
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = false;
			}
			else if (ch == L'\\')
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = true;
			}
			else if ((!littleQuote && (ch == L'"')) || (littleQuote && (ch == L'\'')))
			{
				if (firstQuote)
				{
					preComment = true;
				}
				else
				{
					littleQuote = false;
				}
				quoteMode = false;
				tokenStart = i;
			}
			previ = i;
			firstQuote = false;
			continue;
		}
		else if (preComment)
		{
			preComment = false;
			if ((littleQuote && (ch == L'\'')) || (!littleQuote && (ch == L'"')))
			{
				prevprevi = UINT32_MAX;
				blockComment = true;
				node->syntax[j-2] = colors[tcCOMMENT_BLOCK];
				node->syntax[j-1] = colors[tcCOMMENT_BLOCK];
				node->syntax[j]   = colors[tcCOMMENT_BLOCK];
				previ = i;
			}
			continue;
		}
		else if (ch == L'#')
		{
			comment = true;
			node->syntax[j] = colors[tcCOMMENT_LINE];
			previ = i;
			continue;
		}
		else if (isZero)
		{
			isZero = false;
			if (ch == L'x')
			{
				letter = false;
				hex = true;
				node->syntax[j] = colors[tcHEX];
				previ = i;
				continue;
			}
			if ((ch >= L'0') && (ch <= L'7'))
			{
				octal = true;
			}
		}
		
		switch (ch)
		{
			case L'(':
			case L')':
			case L'{':
			case L'}':
			case L'[':
			case L']':
			case L'.':
			case L',':
			case L'*':
			case L'+':
			case L'-':
			case L'/':
			case L'&':
			case L'|':
			case L'<':
			case L'>':
			case L'=':
			case L';':
			case L':':
				node->syntax[j] = colors[tcPUNCTUATION];
				/* fall through */
			case L' ':
			case L'\t':
				letter = false;
				fStx_checkPyToken(node, tokenStart, previ, colors[tcKEYWORD]);
				tokenStart = i;
				break;
			case L'\'':
				littleQuote = true;
				quoteMode = true;
				firstQuote = true;
				letter = false;
				node->syntax[j] = colors[tcCHARACTER_QUOTE];
				break;
			case L'"':
				quoteMode = true;
				firstQuote = true;
				letter = false;
				node->syntax[j] = colors[tcSTRING_QUOTE];
				break;
			default:
				if (hex)
				{
					const wchar lch = (wchar)towlower(ch);
					if (!letter && (((ch >= L'0') && (ch <= L'9')) || ((lch >= L'a') && (lch <= L'f'))))
					{
						node->syntax[j] = colors[tcNUMBER];
					}
					else
					{
						hex = false;
						tokenStart = i;
					}
				}
				else if (isZero || octal)
				{
					isZero = false;
					if (!letter && ((ch >= L'0') && (ch <= '7')))
					{
						octal = true;
						node->syntax[j] = colors[tcOCT];
					}
					else
					{
						octal = false;
						tokenStart = i;
					}
				}
				else if ((ch >= L'0') && (ch <= L'9'))
				{
					if (!letter)
					{
						isZero = (ch == L'0');
						node->syntax[j] = colors[tcNUMBER];
						tokenStart = i;
					}
				}
				else
				{
					const wchar lch = (wchar)towlower(ch);
					letter = ((lch >= L'a') && (lch <= L'z')) || (ch == L'_');
					if (!letter)
					{
						tokenStart = i;
					}
				}
		}

		previ = i;
	}

	if (!blockComment && !comment && !quoteMode)
	{
		fStx_checkPyToken(node, tokenStart, previ, colors[tcKEYWORD]);
	}

	node->userValue.bits.b1 = blockComment;
	node->userValue.bits.b2 = littleQuote;

	return true;
}

bool fStx_parseJSON(fLine_t * restrict node, const WORD * restrict colors)
{
	if (!fStx_autoAlloc(node))
	{
		return false;
	}

	bool quoteMode = false, littleQuote = false, skip = false, letter = false,
		isZero = false, hex = false, octal = false;
	
	for (u32 i = 0, j = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}
		node->syntax[j] = colors[tcTEXT];


		const wchar ch = node->line[i];
		
		if (quoteMode)
		{
			node->syntax[j] = colors[littleQuote ? tcCHARACTER : tcSTRING];
			if (skip)
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = false;
			}
			else if (ch == L'\\')
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = true;
			}
			else if ((!littleQuote && (ch == L'"')) || (littleQuote && (ch == L'\'')))
			{
				quoteMode = false;
				littleQuote = false;
			}
			continue;
		}
		else if (isZero)
		{
			isZero = false;
			if (ch == L'x')
			{
				letter = false;
				hex = true;
				node->syntax[j] = colors[tcHEX];
				continue;
			}
			if ((ch >= L'0') && (ch <= '7'))
			{
				octal = true;
			}
		}
		
		switch (ch)
		{
			case L'{':
			case L'}':
			case L'[':
			case L']':
			case L'.':
			case L',':
			case L':':
				node->syntax[j] = colors[tcPUNCTUATION];
				/* fall through */
			case L' ':
			case L'\t':
				letter = false;
				break;
			case L'\'':
				littleQuote = true;
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcCHARACTER_QUOTE];
				break;
			case L'"':
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcSTRING_QUOTE];
				break;
			default:
				if (hex)
				{
					const wchar lch = (wchar)towlower(ch);
					if (!letter && (((ch >= L'0') && (ch <= L'9')) || ((lch >= L'a') && (lch <= L'f'))))
					{
						node->syntax[j] = colors[tcNUMBER];
					}
					else
					{
						hex = false;
					}
				}
				else if (isZero || octal)
				{
					isZero = false;
					if (!letter && ((ch >= L'0') && (ch <= '7')))
					{
						octal = true;
						node->syntax[j] = colors[tcOCT];
					}
					else
					{
						octal = false;
					}
				}
				else if ((ch >= L'0') && (ch <= L'9'))
				{
					if (!letter)
					{
						isZero = (ch == L'0');
						node->syntax[j] = colors[tcNUMBER];
					}
				}
				else
				{
					const wchar lch = (wchar)towlower(ch);
					letter = ((lch >= L'a') && (lch <= L'z')) || (ch == L'_');
				}
		}
	}

	return true;
}
bool fStx_parseCSS(fLine_t * restrict node, const WORD * restrict colors)
{
	if (!fStx_autoAlloc(node))
	{
		return false;
	}

	bool quoteMode = false, littleQuote = false, skip = false, letter = false,
		isZero = false, hex = false, octal = false, propertyMode = false,
		valueMode = false, blockComment = false;
	
	propertyMode = (node->prevNode != NULL) ? node->prevNode->userValue.bits.b2 : false;
	blockComment = (node->prevNode != NULL) ? node->prevNode->userValue.bits.b1 : false;

	u32 previ = 0;
	for (u32 i = 0, j = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}
		node->syntax[j] = colors[tcCSS_SELECTOR];


		const wchar ch = node->line[i];
		
		if (blockComment)
		{
			if ((ch == L'/') && (j > 0) && (node->line[previ] == L'*'))
			{
				blockComment = false;
			}
			node->syntax[j] = colors[tcCOMMENT_BLOCK];
			previ = i;
			continue;
		}
		else if (quoteMode)
		{
			node->syntax[j] = colors[littleQuote ? tcCHARACTER : tcSTRING];
			if (skip)
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = false;
			}
			else if (ch == L'\\')
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = true;
			}
			else if ((!littleQuote && (ch == L'"')) || (littleQuote && (ch == L'\'')))
			{
				quoteMode = false;
				littleQuote = false;
			}
			continue;
		}
		else if ((ch == L'*') && (j > 0) && (node->line[previ] == L'/'))
		{
			blockComment = true;
			node->syntax[j-1] = colors[tcCOMMENT_BLOCK];
			node->syntax[j]   = colors[tcCOMMENT_BLOCK];
			previ = i;
			continue;
		}
		else if (isZero)
		{
			isZero = false;
			if (ch == L'x')
			{
				letter = false;
				hex = true;
				node->syntax[j] = colors[tcHEX];
				continue;
			}
			if ((ch >= L'0') && (ch <= '7'))
			{
				octal = true;
			}
		}
		else if (valueMode)
		{
			node->syntax[j] = colors[tcMD_VALUE];

			if (ch == L';')
			{
				valueMode = false;
			}
		}
		else if (propertyMode)
		{
			node->syntax[j] = colors[tcCSS_PROPERTY];
			if (ch == L':')
			{
				valueMode = true;
			}
			else if (ch == L'}')
			{
				propertyMode = false;
				node->syntax[j] = colors[tcPUNCTUATION];
			}
			else
			{
				continue;
			}
		}
		
		switch (ch)
		{
			case L'{':
				propertyMode = true;
				/* fall through */
			case L'.':
			case L',':
			case L':':
			case L';':
				node->syntax[j] = colors[tcPUNCTUATION];
				/* fall through */
			case L' ':
			case L'\t':
				letter = false;
				break;
			case L'\'':
				littleQuote = true;
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcCHARACTER_QUOTE];
				break;
			case L'"':
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcSTRING_QUOTE];
				break;
			default:
				if (hex)
				{
					const wchar lch = (wchar)towlower(ch);
					if (!letter && (((ch >= L'0') && (ch <= L'9')) || ((lch >= L'a') && (lch <= L'f'))))
					{
						node->syntax[j] = colors[tcNUMBER];
					}
					else
					{
						hex = false;
					}
				}
				else if (isZero || octal)
				{
					isZero = false;
					if (!letter && ((ch >= L'0') && (ch <= '7')))
					{
						octal = true;
						node->syntax[j] = colors[tcOCT];
					}
					else
					{
						octal = false;
					}
				}
				else if ((ch >= L'0') && (ch <= L'9'))
				{
					if (!letter)
					{
						isZero = (ch == L'0');
						node->syntax[j] = colors[tcNUMBER];
					}
				}
				else
				{
					const wchar lch = (wchar)towlower(ch);
					letter = ((lch >= L'a') && (lch <= L'z')) || (ch == L'_');
				}
		}

		previ = i;
	}

	node->userValue.bits.b1 = blockComment;
	node->userValue.bits.b2 = propertyMode;

	return true;
}

bool fStx_parseXML(fLine_t * restrict node, const WORD * restrict colors)
{
	if (!fStx_autoAlloc(node))
	{
		return false;
	}

	bool quoteMode = false, littleQuote = false, skip = false, letter = false,
		isZero = false, hex = false, octal = false, tagMode = false, firstTag = false,
		tagEnd = false, blockComment = false, specialTag = false, value = false,
		escapeChar = false;

	blockComment = (node->prevNode != NULL) ? node->prevNode->userValue.bits.b1 : false;

	u32 comm[5] = { 0 };
	
	for (u32 i = 0, j = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}
		node->syntax[j] = colors[tcTEXT];


		const wchar ch = node->line[i];
		
		if (quoteMode)
		{
			node->syntax[j] = colors[littleQuote ? tcCHARACTER : tcSTRING];
			if (skip)
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = false;
			}
			else if (ch == L'\\')
			{
				node->syntax[j] = colors[tcESCAPE];
				skip = true;
			}
			else if ((!littleQuote && (ch == L'"')) || (littleQuote && (ch == L'\'')))
			{
				quoteMode = false;
				littleQuote = false;
			}
			continue;
		}
		else if (isZero)
		{
			isZero = false;
			if (ch == L'x')
			{
				letter = false;
				hex = true;
				node->syntax[j] = colors[tcHEX];
				continue;
			}
			if ((ch >= L'0') && (ch <= '7'))
			{
				octal = true;
			}
		}
		else if (blockComment)
		{
			node->syntax[j] = colors[tcCOMMENT_BLOCK];
			if (ch == L'-')
			{
				if (comm[3] == 0)
				{
					comm[0] = j;
					comm[3] = 1;
				}
				else if (comm[4] == 0)
				{
					comm[1] = j;
					comm[4] = 1;
				}
			}
			else if ((ch == '>') && comm[3] && comm[4])
			{
				blockComment = false;
			}
			else
			{
				comm[3] = comm[4] = 0;
			}
			continue;
		}
		else if (escapeChar)
		{
			node->syntax[j] = colors[tcESCAPE];
			if (ch == L';')
			{
				escapeChar = false;
			}
			continue;
		}
		else if (specialTag)
		{
			node->syntax[j] = colors[tcKEYWORD];
			if (ch == L'?')
			{
				specialTag = false;
			}
		}
		else if (tagEnd)
		{
			node->syntax[j] = colors[tcKEYWORD];
			if (ch == '>')
			{
				tagEnd = false;
				tagMode = false;
			}
		}
		else if (tagMode)
		{
			node->syntax[j] = colors[value ? tcXML_ID : tcKEYWORD];
			if (firstTag)
			{
				firstTag = false;
				if (ch == L'?')
				{
					specialTag = true;
				}
				else if (ch == L'/')
				{
					tagEnd = true;
				}
				else if (ch == L'!')
				{
					comm[1] = j;
					comm[3] = 1;
				}
			}
			else
			{
				if (ch == L'-')
				{
					if (comm[3] && comm[4])
					{
						blockComment = true;
						node->syntax[j]       = colors[tcCOMMENT_BLOCK];
						node->syntax[comm[2]] = colors[tcCOMMENT_BLOCK];
						node->syntax[comm[1]] = colors[tcCOMMENT_BLOCK];
						node->syntax[comm[0]] = colors[tcCOMMENT_BLOCK];

						comm[3] = comm[4] = 0;
					}
					else
					{
						comm[2] = j;
						comm[4] = 1;
					}
				}
				else if (ch == L'>')
				{
					tagMode = false;
					node->syntax[j] = colors[tcKEYWORD];
				}
				else
				{
					comm[3] = comm[4] = 0;
				}
			}
		}
		else if (ch == L'<')
		{
			tagMode  = true;
			firstTag = true;
			comm[0]  = j;
			node->syntax[j] = colors[tcKEYWORD];
			value = false;
		}
		
		switch (ch)
		{
			case '&':
				node->syntax[j] = colors[tcESCAPE];
				escapeChar = true;
				break;
			case L'.':
			case L',':
			case L'=':
				node->syntax[j] = colors[tcPUNCTUATION];
				/* fall through */
			case L' ':
			case L'\t':
				letter = false;
				value = true;
				break;
			case L'\'':
				littleQuote = true;
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcCHARACTER_QUOTE];
				break;
			case L'"':
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcSTRING_QUOTE];
				break;
			default:
				if (hex)
				{
					const wchar lch = (wchar)towlower(ch);
					if (!letter && (((ch >= L'0') && (ch <= L'9')) || ((lch >= L'a') && (lch <= L'f'))))
					{
						node->syntax[j] = colors[tcNUMBER];
					}
					else
					{
						hex = false;
					}
				}
				else if (isZero || octal)
				{
					isZero = false;
					if (!letter && ((ch >= L'0') && (ch <= '7')))
					{
						octal = true;
						node->syntax[j] = colors[tcOCT];
					}
					else
					{
						octal = false;
					}
				}
				else if ((ch >= L'0') && (ch <= L'9'))
				{
					if (!letter)
					{
						isZero = (ch == L'0');
						node->syntax[j] = colors[tcNUMBER];
					}
				}
				else
				{
					const wchar lch = (wchar)towlower(ch);
					letter = ((lch >= L'a') && (lch <= L'z')) || (ch == L'_');
				}
		}
	}

	node->userValue.bits.b1 = blockComment;

	return true;
}


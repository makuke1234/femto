#include "femtoSyntax.h"
#include "femtoLine.h"
#include "femtoSettings.h"


const char * fSyntaxName(enum femtoSyntax fs)
{
	static const char * syntaxes[fstxSIZE] = {
		"None",
		"C",
		"C++",
		"Markdown"
	};

	assert(fs <= fstxSIZE);
	return syntaxes[fs];
}
bool fSyntaxParseAutoAlloc(femtoLineNode_t * restrict node)
{
	assert(node != NULL);

	void * mem = realloc(node->syntax, sizeof(WORD) * node->lineEndx);
	if (mem == NULL)
	{
		free(node->syntax);
		node->syntax = NULL;
		return false;
	}
	node->syntax = mem;

	return true;
}

void checkCToken(femtoLineNode_t * restrict node, uint32_t start, uint32_t lasti, WORD kwCol)
{
	if ((lasti - start) < 1)
	{
		return;
	}

	struct keyword
	{
		uint8_t len;
		const wchar_t ** words;
	};

	static const wchar_t * keywordsLen2[] = {
		L"do",
		L"if"
	};
	static const wchar_t * keywordsLen3[] = {
		L"for",
		L"int"
	};
	static const wchar_t * keywordsLen4[] = {
		L"auto",
		L"case",
		L"char",
		L"else",
		L"enum",
		L"long",
		L"void",
		L"goto",

		L"bool"
	};
	static const wchar_t * keywordsLen5[] = {
		L"break",
		L"const",
		L"float",
		L"short",
		L"union",
		L"while",

		L"_Bool"
	};
	static const wchar_t * keywordsLen6[] = {
		L"double",
		L"extern",
		L"inline",
		L"return",
		L"signed",
		L"sizeof",
		L"static",
		L"struct",
		L"switch"
	};
	static const wchar_t * keywordsLen7[] = {
		L"default",

		L"_Atomic",

		L"alignas",
		L"alignof",
		L"complex",
		L"typedef"
	};
	static const wchar_t * keywordsLen8[] = {
		L"continue",
		L"register",
		L"restrict",
		L"unsigned",
		L"volatile",

		L"_Alignas",
		L"_Alignof",
		L"_Complex",
		L"_Generic",

		L"noreturn"
	};
	static const wchar_t * keywordsLen9[] = {
		L"_Noreturn",

		L"imaginary"
	};
	static const wchar_t * keywordsLen10[] = {
		L"_Decimal64",
		L"_Decimal32",
		L"_Imaginary"
	};
	static const wchar_t * keywordsLen11[] = {
		L"_Decimal128"
	};
	static const wchar_t * keywordsLen12[] = {
		L"thread_local"
	};
	static const wchar_t * keywordsLen13[] = {
		L"_Thread_local",

		L"static_assert"
	};
	static const wchar_t * keywordsLen14[] = {
		L"_Static_assert"
	};

	static const struct keyword words[] = {
		[2]  = { .len = ARRAYSIZE(keywordsLen2),  .words = keywordsLen2  },
		[3]  = { .len = ARRAYSIZE(keywordsLen3),  .words = keywordsLen3  },
		[4]  = { .len = ARRAYSIZE(keywordsLen4),  .words = keywordsLen4  },
		[5]  = { .len = ARRAYSIZE(keywordsLen5),  .words = keywordsLen5  },
		[6]  = { .len = ARRAYSIZE(keywordsLen6),  .words = keywordsLen6  },
		[7]  = { .len = ARRAYSIZE(keywordsLen7),  .words = keywordsLen7  },
		[8]  = { .len = ARRAYSIZE(keywordsLen8),  .words = keywordsLen8  },
		[9]  = { .len = ARRAYSIZE(keywordsLen9),  .words = keywordsLen9  },
		[10] = { .len = ARRAYSIZE(keywordsLen10), .words = keywordsLen10 },
		[11] = { .len = ARRAYSIZE(keywordsLen11), .words = keywordsLen11 },
		[12] = { .len = ARRAYSIZE(keywordsLen12), .words = keywordsLen12 },
		[13] = { .len = ARRAYSIZE(keywordsLen13), .words = keywordsLen13 },
		[14] = { .len = ARRAYSIZE(keywordsLen14), .words = keywordsLen14 }
	};
	static const uint32_t wordLens[] = {
		14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2
	};
	static const size_t nwordLens = ARRAYSIZE(wordLens);

	wchar_t kwBuf[MAX_KWBUF];
	kwBuf[MAX_KWBUF - 1] = L'\0';

	uint32_t filled = 0;
	for (int32_t i = MAX_KWBUF - 2, j = (int32_t)lasti, starti = (int32_t)start; (i >= 0) && (j >= 0) && (j >= starti);)
	{
		if (((j - (int32_t)node->freeSpaceLen) == ((int32_t)node->curx - 1)) && (node->freeSpaceLen > 0))
		{
			j -= (int32_t)node->freeSpaceLen;
			continue;
		}
		kwBuf[i] = node->line[j];
		--i;
		--j;
		++filled;
	}

	for (uint32_t i = 0; i < nwordLens; ++i)
	{
		const uint32_t n = wordLens[nwordLens - i - 1];
		if ((n > filled) || ((filled - n) > 2))
		{
			continue;
		}
		for (uint32_t j = 0; j < words[n].len; ++j)
		{
			if (wcscmp(kwBuf + (MAX_KWBUF - 1) - n, words[n].words[j]) == 0)
			{
				int32_t l = (lasti > node->curx) ? (int32_t)(lasti - node->freeSpaceLen) : (int32_t)lasti;
				for (uint32_t k = 0; k < n; --l, ++k)
				{
					node->syntax[l] = kwCol;
				}
				return;
			}
		}
	}
}
void checkCPPToken(femtoLineNode_t * restrict node, uint32_t start, uint32_t lasti, WORD kwCol)
{
	if ((lasti - start) < 1)
	{
		return;
	}

	struct keyword
	{
		uint8_t len;
		const wchar_t ** words;
	};

	static const wchar_t * keywordsLen2[] = {
		L"do",
		L"if",
		L"or"
	};
	static const wchar_t * keywordsLen3[] = {
		L"for",
		L"int",
		L"and",
		L"asm",
		L"new",
		L"not",
		L"xor",
		L"try"
	};
	static const wchar_t * keywordsLen4[] = {
		L"auto",
		L"case",
		L"char",
		L"else",
		L"enum",
		L"long",
		L"void",
		L"goto",
		L"this",
		L"bool",
		L"true"
	};
	static const wchar_t * keywordsLen5[] = {
		L"break",
		L"const",
		L"float",
		L"short",
		L"union",
		L"while",
		L"bitor",
		L"class",
		L"catch",
		L"compl",
		L"false",
		L"or_eq",
		L"throw",
		L"using",
		L"final"
	};
	static const wchar_t * keywordsLen6[] = {
		L"double",
		L"extern",
		L"inline",
		L"return",
		L"signed",
		L"sizeof",
		L"static",
		L"struct",
		L"switch",
		L"and_eq",
		L"bitand",
		L"delete",
		L"export",
		L"friend",
		L"not_eq",
		L"public",
		L"typeid",
		L"xor_eq",
		L"import",
		L"module"
	};
	static const wchar_t * keywordsLen7[] = {
		L"alignas",
		L"alignof",
		L"char8_t",
		L"concept",
		L"default",
		L"mutable",
		L"nullptr",
		L"private",
		L"typedef",
		L"virtual",
		L"wchar_t"
	};
	static const wchar_t * keywordsLen8[] = {
		L"char16_t",
		L"char32_t",
		L"continue",
		L"co_await",
		L"co_yield",
		L"decltype",
		L"explicit",
		L"noexcept",
		L"operator",
		L"register",
		L"requires",
		L"template",
		L"typename",
		L"unsigned",
		L"volatile",
		L"override"
	};
	static const wchar_t * keywordsLen9[] = {
		L"consteval",
		L"constexpr",
		L"constinit",
		L"co_return",
		L"namespace",
		L"protected"
	};
	static const wchar_t * keywordsLen10[] = {
		L"const_cast"
	};
	static const wchar_t * keywordsLen11[] = {
		L"static_cast"
	};
	static const wchar_t * keywordsLen12[] = {
		L"dynamic_cast",
		L"thread_local"
	};
	static const wchar_t * keywordsLen13[] = {
		L"static_assert"
	};
	static const wchar_t * keywordsLen16[] = {
		L"reinterpret_cast"
	};

	static const struct keyword words[] = {
		[2]  = { .len = ARRAYSIZE(keywordsLen2),  .words = keywordsLen2  },
		[3]  = { .len = ARRAYSIZE(keywordsLen3),  .words = keywordsLen3  },
		[4]  = { .len = ARRAYSIZE(keywordsLen4),  .words = keywordsLen4  },
		[5]  = { .len = ARRAYSIZE(keywordsLen5),  .words = keywordsLen5  },
		[6]  = { .len = ARRAYSIZE(keywordsLen6),  .words = keywordsLen6  },
		[7]  = { .len = ARRAYSIZE(keywordsLen7),  .words = keywordsLen7  },
		[8]  = { .len = ARRAYSIZE(keywordsLen8),  .words = keywordsLen8  },
		[9]  = { .len = ARRAYSIZE(keywordsLen9),  .words = keywordsLen9  },
		[10] = { .len = ARRAYSIZE(keywordsLen10), .words = keywordsLen10 },
		[11] = { .len = ARRAYSIZE(keywordsLen11), .words = keywordsLen11 },
		[12] = { .len = ARRAYSIZE(keywordsLen12), .words = keywordsLen12 },
		[13] = { .len = ARRAYSIZE(keywordsLen13), .words = keywordsLen13 },
		[16] = { .len = ARRAYSIZE(keywordsLen16), .words = keywordsLen16 }
	};
	static const uint32_t wordLens[] = {
		16, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2
	};
	static const size_t nwordLens = ARRAYSIZE(wordLens);

	wchar_t kwBuf[MAX_KWBUF];
	kwBuf[MAX_KWBUF - 1] = L'\0';

	uint32_t filled = 0;
	for (int32_t i = MAX_KWBUF - 2, j = (int32_t)lasti, starti = (int32_t)start; (i >= 0) && (j >= 0) && (j >= starti);)
	{
		if (((j - (int32_t)node->freeSpaceLen) == ((int32_t)node->curx - 1)) && (node->freeSpaceLen > 0))
		{
			j -= (int32_t)node->freeSpaceLen;
			continue;
		}
		kwBuf[i] = node->line[j];
		--i;
		--j;
		++filled;
	}

	for (uint32_t i = 0; i < nwordLens; ++i)
	{
		const uint32_t n = wordLens[nwordLens - i - 1];
		if ((n > filled) || ((filled - n) > 2))
		{
			continue;
		}

		for (uint32_t j = 0; j < words[n].len; ++j)
		{
			if (wcscmp(kwBuf + (MAX_KWBUF - 1) - n, words[n].words[j]) == 0)
			{
				int32_t l = (lasti > node->curx) ? (int32_t)(lasti - node->freeSpaceLen) : (int32_t)lasti;
				for (uint32_t k = 0; k < n; --l, ++k)
				{
					node->syntax[l] = kwCol;
				}
				return;
			}
		}
	}
}

bool fSyntaxParseNone(struct femtoLineNode * restrict node, const WORD * restrict colors)
{
	if (!fSyntaxParseAutoAlloc(node))
	{
		return false;
	}

	for (uint32_t i = 0, j = 0; i < node->lineEndx; ++i, ++j)
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
bool fSyntaxParseCLike(femtoLineNode_t * restrict node, const WORD * restrict colors, tokeniserFunc_t func)
{
	if (!fSyntaxParseAutoAlloc(node))
	{
		return false;
	}

	uint32_t tokenStart = 0;
	bool quoteMode = false, littleQuote = false, skip = false, letter = false,
		isZero = false, hex = false, octal = false, comment = false, blockComment = false, preproc = false;
	
	blockComment = (node->prevNode != NULL) ? node->prevNode->bBlockComment : false;

	uint32_t previ = 0;
	for (uint32_t i = 0, j = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}
		node->syntax[j] = colors[tcTEXT];


		const wchar_t ch = node->line[i];
		
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
		else if (preproc)
		{
			node->syntax[j] = colors[tcPREPROC];
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
				/* fall through */
			case L'"':
				quoteMode = true;
				letter = false;
				node->syntax[j] = colors[tcSTRING_QUOTE];
				break;
			case L'#':
				preproc = true;
				break;
			default:
				if (hex)
				{
					const wchar_t lch = (wchar_t)towlower(ch);
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
					const wchar_t lch = (wchar_t)towlower(ch);
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

	node->bBlockComment = blockComment;

	return true;
}
bool fSyntaxParseMd(femtoLineNode_t * restrict node, const WORD * restrict colors)
{
	if (!fSyntaxParseAutoAlloc(node))
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
	
	uint32_t firstChIdx = 0;
	bool done = false, headingMode = false, valueMode = false, bracketMode = false,
		extraBracketMode = false, italicsMode = false, boldMode = false,
		containsStar = false, strikeMode = false, parenMode1 = false,
		parenMode2 = false, enable = false, coneMode = false, codeMode = false;
	
	for (uint32_t i = 0, j = 0, previ = 0; i < node->lineEndx; ++i, ++j)
	{
		if ((i == node->curx) && (node->freeSpaceLen > 0))
		{
			i += node->freeSpaceLen;
			--i;
			--j;
			continue;
		}
		node->syntax[j] = colors[tcTEXT];

		const wchar_t ch = node->line[i];
		
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

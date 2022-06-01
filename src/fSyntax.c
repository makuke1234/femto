#include "fSyntax.h"
#include "fLine.h"
#include "fSettings.h"
#include "fFile.h"
#include "fStatHashmap.h"


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

void fStx_checkCToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_C_TOKEN_MEM];

	static const wchar * keyWords[] = {
		L"do",
		L"if",
		L"for",
		L"int",
		L"auto",
		L"case",
		L"char",
		L"else",
		L"enum",
		L"long",
		L"void",
		L"goto",
		L"bool",
		L"break",
		L"const",
		L"float",
		L"short",
		L"union",
		L"while",
		L"_Bool",
		L"double",
		L"extern",
		L"inline",
		L"return",
		L"signed",
		L"sizeof",
		L"static",
		L"struct",
		L"switch",
		L"default",
		L"_Atomic",
		L"alignas",
		L"alignof",
		L"complex",
		L"typedef",
		L"continue",
		L"register",
		L"restrict",
		L"unsigned",
		L"volatile",
		L"_Alignas",
		L"_Alignof",
		L"_Complex",
		L"_Generic",
		L"noreturn",
		L"_Noreturn",
		L"imaginary",
		L"_Decimal64",
		L"_Decimal32",
		L"_Imaginary",
		L"_Decimal128",
		L"thread_local",
		L"_Thread_local",
		L"static_assert",
		L"_Static_assert",

		// Common types
		L"int8_t",
		L"int16_t",
		L"int32_t",
		L"int64_t",
		L"wchar_t",
		
		L"uint8_t",
		L"uint16_t",
		L"uint32_t",
		L"uint64_t",

		L"intptr_t",
		L"uintptr_t",
		L"size_t",
		L"intmax_t",
		L"uintmax_t",

		L"int_least8_t",
		L"int_least16_t",
		L"int_least32_t",
		L"int_least64_t",
		
		L"uint_least8_t",
		L"uint_least16_t",
		L"uint_least32_t",
		L"uint_least64_t",

		L"int_fast8_t",
		L"int_fast16_t",
		L"int_fast32_t",
		L"int_fast64_t",

		L"uint_fast8_t",
		L"uint_fast16_t",
		L"uint_fast32_t",
		L"uint_fast64_t",

		L"FILE",
		L"time_t",
		L"clock_t",
		L"timespec",
		L"tm",

		L"va_start",
		L"va_arg",
		L"va_copy",
		L"va_end",
		L"va_list",
		
		L"scanf",
		L"fscanf"
		L"sscanf",
		L"scanf_s",
		L"fscanf_s",
		L"sscanf_s",

		L"printf",
		L"fprintf",
		L"sprintf",
		L"snprintf",
		L"printf_s",
		L"fprintf_s",
		L"sprintf_s",
		L"snprintf_s",

		L"time",
		L"srand",
		L"rand",
		L"open",
		L"close",
		L"fopen",
		L"fclose",

		L"NULL",
		L"freopen",
		L"fflush",
		L"setbuf",
		L"setvbuf",
		L"fread",
		L"fwrite",
		L"fgetc",
		L"getc",
		L"fgets",
		L"fputc",
		L"putc",
		L"fputs",
		L"getchar",
		L"putchar",
		L"puts",
		L"ungetc",
		L"vscanf",
		L"vfscanf",
		L"vsscanf",
		L"vprintf",
		L"vfprintf",
		L"vsprintf",
		L"vsnprintf",
		L"ftell",
		L"fgetpos",
		L"fseek",
		L"fsetpos",
		L"rewind",
		L"clearerr",
		L"feof",
		L"ferror",
		L"perror",
		L"remove",
		L"rename",
		L"tmpfile",
		L"tmpnam",

		L"malloc",
		L"free",
		L"realloc",
		L"calloc",

		L"qsort",
		L"abort",
		L"exit",
		L"atexit",
		L"system",
		L"getenv",
		L"atof",
		L"atoi",
		L"bsearch",
		L"abs",

		L"memcpy",
		L"strcpy",
		L"strcat",
		L"strlen",

		L"roundf",
		L"round",
		L"roundl",
		L"lroundf",
		L"lround",
		L"lroundl",
		L"llroundf",
		L"llround",
		L"llroundl",

		L"sinf",
		L"sin",
		L"sinl",
		L"cosf",
		L"cos",
		L"cosl",
		L"tanf",
		L"tan",
		L"tanl",

		L"asinf",
		L"asin",
		L"asinl",
		L"acosf",
		L"acos",
		L"acosl",
		L"atanf",
		L"atan",
		L"atanl",

		L"atan2f",
		L"atan2",
		L"atan2l"

	};
	static fStatHash_t map = { 0 };
	
	if ((lasti - start) < 1)
	{
		return;
	}

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_C_TOKEN_MEM, keyWords, ARRAYSIZE(keyWords));

	wchar kwBuf[MAX_KWBUF];
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
		else if (fStatHash_get(&map, kwBuf + (MAX_KWBUF - 1) - n))
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
void fStx_checkCPPToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_CPP_TOKEN_MEM];

	static const wchar * keyWords[] = {
		L"do",
		L"if",
		L"or",
		L"for",
		L"int",
		L"and",
		L"asm",
		L"new",
		L"not",
		L"xor",
		L"try",
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
		L"true",
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
		L"final",
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
		L"module",
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
		L"wchar_t",
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
		L"override",
		L"consteval",
		L"constexpr",
		L"constinit",
		L"co_return",
		L"namespace",
		L"protected",
		L"const_cast",
		L"static_cast",
		L"dynamic_cast",
		L"thread_local",
		L"static_assert",
		L"reinterpret_cast",

		// Common types
		L"int8_t",
		L"int16_t",
		L"int32_t",
		L"int64_t",
		
		L"uint8_t",
		L"uint16_t",
		L"uint32_t",
		L"uint64_t",

		L"intptr_t",
		L"uintptr_t",
		L"size_t",
		L"intmax_t",
		L"uintmax_t",

		L"int_least8_t",
		L"int_least16_t",
		L"int_least32_t",
		L"int_least64_t",
		
		L"uint_least8_t",
		L"uint_least16_t",
		L"uint_least32_t",
		L"uint_least64_t",

		L"int_fast8_t",
		L"int_fast16_t",
		L"int_fast32_t",
		L"int_fast64_t",

		L"uint_fast8_t",
		L"uint_fast16_t",
		L"uint_fast32_t",
		L"uint_fast64_t",

		L"FILE",
		L"time_t",
		L"clock_t",
		L"timespec",
		L"tm",

		L"va_start",
		L"va_arg",
		L"va_copy",
		L"va_end",
		L"va_list",
		
		L"scanf",
		L"fscanf"
		L"sscanf",
		L"scanf_s",
		L"fscanf_s",
		L"sscanf_s",

		L"printf",
		L"fprintf",
		L"sprintf",
		L"snprintf",
		L"printf_s",
		L"fprintf_s",
		L"sprintf_s",
		L"snprintf_s",

		L"time",
		L"srand",
		L"rand",
		L"open",
		L"close",
		L"fopen",
		L"fclose",

		L"NULL",
		L"freopen",
		L"fflush",
		L"setbuf",
		L"setvbuf",
		L"fread",
		L"fwrite",
		L"fgetc",
		L"getc",
		L"fgets",
		L"fputc",
		L"putc",
		L"fputs",
		L"getchar",
		L"putchar",
		L"puts",
		L"ungetc",
		L"vscanf",
		L"vfscanf",
		L"vsscanf",
		L"vprintf",
		L"vfprintf",
		L"vsprintf",
		L"vsnprintf",
		L"ftell",
		L"fgetpos",
		L"fseek",
		L"fsetpos",
		L"rewind",
		L"clearerr",
		L"feof",
		L"ferror",
		L"perror",
		L"remove",
		L"rename",
		L"tmpfile",
		L"tmpnam",

		L"malloc",
		L"free",
		L"realloc",
		L"calloc",

		L"qsort",
		L"abort",
		L"exit",
		L"atexit",
		L"system",
		L"getenv",
		L"atof",
		L"atoi",
		L"bsearch",
		L"abs",

		L"memcpy",
		L"strcpy",
		L"strcat",
		L"strlen",

		// C++ common types
		L"array",
		L"string",
		L"vector",
		L"cout",
		L"cin",
		L"basic_iostream",
		L"istream",
		L"ostream",
		L"stringstream",
		L"istringstream",
		L"ostringstream",
		L"wstring",
		L"basic_string",
		L"basic_string_view",
		L"string_view",
		L"wstring_view",
		L"map",
		L"unordered_map",
		L"set",
		L"unordered_set",

		L"atol",
		L"atoll",
		L"strtol",
		L"strtoll",
		L"strtoul",
		L"strtoull",
		L"strtof",
		L"strtod",
		L"strtold",
		L"labs",
		L"llabs",
		L"div",
		L"ldiv",
		L"lldiv",

		L"roundf",
		L"round",
		L"roundl",
		L"lroundf",
		L"lround",
		L"lroundl",
		L"llroundf",
		L"llround",
		L"llroundl",

		L"sinf",
		L"sin",
		L"sinl",
		L"cosf",
		L"cos",
		L"cosl",
		L"tanf",
		L"tan",
		L"tanl",

		L"asinf",
		L"asin",
		L"asinl",
		L"acosf",
		L"acos",
		L"acosl",
		L"atanf",
		L"atan",
		L"atanl",

		L"atan2f",
		L"atan2",
		L"atan2l",

		L"endl",
		L"flush",
		L"push_back",
		L"emplace_back",
		L"at",
		L"substr",
		L"front",
		L"back",
		L"begin",
		L"end",
		L"rbegin",
		L"rend",
		L"size",
		L"empty",
		L"max_size",
		L"reserve",
		L"capacity",
		L"shrink_to_fit",
		L"clear",
		L"insert",
		L"erase",
		L"pop_back",
		L"append",
		L"compare",
		L"replace",
		L"find",
		L"rfind",
		L"swap",
		L"resize",
		L"stoi",
		L"stol",
		L"stoll",
		L"stoul",
		L"stoull",
		L"stof",
		L"stod",
		L"stold",
		L"to_string",
		L"to_wstring",
		L"hash",
		L"data",
		L"c_str",
		L"get_allocator",
		L"starts_with",
		L"ends_with",
		L"copy",
		L"emplace_hint",
		L"emplace",
		L"insert_or_assign",
		L"try_emplace",
		L"extract",
		L"merge",
		L"contains",
		L"equal_range",
		L"lower_bound",
		L"upper_bound",
		L"erase_if",
		L"count",
		L"bucket_count",
		L"max_bucket_count",
		L"load_factor",
		L"max_load_factor",
		L"rehash",
		L"bucket"
	};
	static fStatHash_t map = { 0 };


	if ((lasti - start) < 1)
	{
		return;
	}

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_CPP_TOKEN_MEM, keyWords, ARRAYSIZE(keyWords));

	wchar kwBuf[MAX_KWBUF];
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

	for (u32 i = 0; i < (MAX_CPP_TOKEN_WORD - 1); ++i)
	{
		const u32 n = MAX_CPP_TOKEN_WORD - i;
		if ((n > filled) || ((filled - n) > 1))
		{
			continue;
		}
		else if (fStatHash_get(&map, kwBuf + (MAX_KWBUF - 1) - n))
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
void fStx_checkPyToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_PY_TOKEN_MEM];

	static const wchar * keyWords[] = {
		L"and",
		L"as",
		L"assert",
		L"break",
		L"class",
		L"continue",
		L"def",
		L"del",
		L"elif",
		L"else",
		L"except",
		L"False",
		L"finally",
		L"for",
		L"from",
		L"global",
		L"if",
		L"import",
		L"in",
		L"is",
		L"lambda",
		L"None",
		L"nonlocal",
		L"not",
		L"or",
		L"pass",
		L"raise",
		L"return",
		L"True",
		L"while",
		L"with",
		L"yield",

		L"abs",
		L"all",
		L"any",
		L"ascii",
		L"bin",
		L"bool",
		L"bytearray",
		L"bytes",
		L"callable",
		L"chr",
		L"classmethod",
		L"compile",
		L"complex",
		L"delattr",
		L"dict",
		L"dir",
		L"divmod",
		L"enumerate",
		L"eval",
		L"exec",
		L"filter",
		L"float",
		L"format",
		L"frozenset",
		L"getattr",
		L"globals",
		L"hasattr",
		L"hash",
		L"help",
		L"hex",
		L"id",
		L"input",
		L"int",
		L"isinstance",
		L"issubclass",
		L"iter",
		L"len",
		L"list",
		L"locals",
		L"map",
		L"max",
		L"memoryview",
		L"min",
		L"next",
		L"object",
		L"oct",
		L"open",
		L"ord",
		L"pow",
		L"print",
		L"property",
		L"range",
		L"reversed",
		L"round",
		L"set",
		L"setattr",
		L"slice",
		L"sorted",
		L"staticmethod",
		L"str",
		L"sum",
		L"super",
		L"tuple",
		L"type",
		L"vars",
		L"zip",

		L"ArithmeticError",
		L"AssertionError",
		L"AttributeError",
		L"Exception",
		L"EOFError",
		L"FloatingPointError",
		L"GeneratorExit",
		L"ImportError",
		L"IndendationError",
		L"IndexError",
		L"KeyError",
		L"KeyboardInterrupt",
		L"LookupError",
		L"MemoryError",
		L"NameError",
		L"NotImplementedError",
		L"OSError",
		L"OverflowError",
		L"ReferenceError",
		L"RuntimeError",
		L"StopIteration",
		L"SyntaxError",
		L"TabError",
		L"SystemError",
		L"SystemExit",
		L"TypeError",
		L"UnboundLocalError",
		L"UnicodeError",
		L"UnicodeEncodeError",
		L"UnicodeDecodeError",
		L"UnicodeTranslateError",
		L"ValueError",
		L"ZeroDivisionError"
	};
	static fStatHash_t map = { 0 };


	if ((lasti - start) < 1)
	{
		return;
	}

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_PY_TOKEN_MEM, keyWords, ARRAYSIZE(keyWords));

	wchar kwBuf[MAX_KWBUF];
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

	for (u32 i = 0; i < (MAX_PY_TOKEN_WORD - 1); ++i)
	{
		const u32 n = MAX_PY_TOKEN_WORD - i;
		if ((n > filled) || ((filled - n) > 1))
		{
			continue;
		}
		else if (fStatHash_get(&map, kwBuf + (MAX_KWBUF - 1) - n))
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
void fStx_checkJSToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_JS_TOKEN_MEM];

	static const wchar * keyWords[] = {
		L"abstract",
		L"arguments",
		L"await",
		L"boolean",
		L"break",
		L"byte",
		L"case",
		L"catch",
		L"char",
		L"class",
		L"const",
		L"continue",
		L"debugger",
		L"default",
		L"delete",
		L"do",
		L"double",
		L"else",
		L"enum",
		L"eval",
		L"export",
		L"extends",
		L"false",
		L"final",
		L"finally",
		L"float",
		L"for",
		L"function",
		L"goto",
		L"if",
		L"implements",
		L"import",
		L"in",
		L"instanceof",
		L"int",
		L"interface",
		L"let",
		L"long",
		L"native",
		L"new",
		L"null",
		L"package",
		L"private",
		L"protected",
		L"public",
		L"return",
		L"short",
		L"static",
		L"super",
		L"switch",
		L"synchronized",
		L"this",
		L"throw",
		L"throws",
		L"transient",
		L"true",
		L"try",
		L"typeof",
		L"var",
		L"void",
		L"volatile",
		L"while",
		L"with",
		L"yield",

		L"Array",
		L"Date",
		L"hasOwnProperty",
		L"Infinity",
		L"isFinite",
		L"isNaN",
		L"isPrototypeOf",
		L"length",
		L"Math",
		L"NaN",
		L"name",
		L"Number",
		L"Object",
		L"prototype",
		L"String",
		L"toString",
		L"undefined",
		L"valueOf",
		
		L"alert",
		L"all",
		L"anchor",
		L"anchors",
		L"area",
		L"assign",
		L"blur",
		L"button",
		L"checkbox",
		L"clearInterval",
		L"clearTimeout",
		L"clientInformation",
		L"close",
		L"closed",
		L"confirm",
		L"constructor",
		L"crypto",
		L"decodeURI",
		L"decodeURIComponent",
		L"defaultStatus",
		L"document",
		L"element",
		L"elements",
		L"embed",
		L"embeds",
		L"encodeURI",
		L"encodeURIComponent",
		L"escape",
		L"event",
		L"fileUpload",
		L"focus",
		L"form",
		L"forms",
		L"frame",
		L"innerHeight",
		L"innerWidth",
		L"layer",
		L"layers",
		L"link",
		L"location",
		L"mimeTypes",
		L"navigate",
		L"navigator",
		L"frames",
		L"frameRate",
		L"hidden",
		L"history",
		L"image",
		L"images",
		L"offscreenBuffering",
		L"open",
		L"opener",
		L"option",
		L"outerHeight",
		L"outerWidth",
		L"packages",
		L"pageXOffset",
		L"pageYOffset",
		L"parent",
		L"parseFloat",
		L"parseInt",
		L"password",
		L"pkcs11",
		L"plugin",
		L"prompt",
		L"propertyIsEnum",
		L"radio",
		L"reset",
		L"screenX",
		L"screenY",
		L"scroll",
		L"secure",
		L"select",
		L"self",
		L"setInterval",
		L"setTimeout",
		L"status",
		L"submit",
		L"taint",
		L"text",
		L"textArea",
		L"top",
		L"unescape",
		L"untaint",
		L"window",

		L"onblur",
		L"onclick",
		L"onerror",
		L"onfocus",
		L"onkeydown",
		L"onkeypress",
		L"onkeyup",
		L"onmouseover",
		L"onload",
		L"onmouseup",
		L"onmousedown",
		L"onsubmit"
	};

	static fStatHash_t map = { 0 };


	if ((lasti - start) < 1)
	{
		return;
	}

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_JS_TOKEN_MEM, keyWords, ARRAYSIZE(keyWords));

	wchar kwBuf[MAX_KWBUF];
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

	for (u32 i = 0; i < (MAX_JS_TOKEN_WORD - 1); ++i)
	{
		const u32 n = MAX_JS_TOKEN_WORD - i;
		if ((n > filled) || ((filled - n) > 2))
		{
			continue;
		}
		else if (fStatHash_get(&map, kwBuf + (MAX_KWBUF - 1) - n))
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
void fStx_checkRustToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_RUST_TOKEN_MEM];

	// Rust keywords
	static const wchar * keyWords[] = {
		L"as",
		L"break",
		L"const",
		L"continue",
		L"crate",
		L"else",
		L"enum",
		L"extern",
		L"false",
		L"fn",
		L"for",
		L"if",
		L"impl",
		L"in",
		L"let",
		L"loop",
		L"match",
		L"mod",
		L"move",
		L"mut",
		L"pub",
		L"ref",
		L"return",
		L"self",
		L"Self",
		L"static",
		L"struct",
		L"super",
		L"trait",
		L"true",
		L"type",
		L"unsafe",
		L"use",
		L"where",
		L"while",

		L"async",
		L"await",
		L"dyn",

		L"abstract",
		L"become",
		L"box",
		L"do",
		L"final",
		L"macro",
		L"override",
		L"priv",
		L"typeof",
		L"unsized",
		L"virtual",
		L"yield",

		L"try",

		L"union",

		// Built-in types
		L"bool"
		L"char",
		L"str",

		L"i8",
		L"u8",
		L"i16",
		L"u16",
		L"i32",
		L"u32",
		L"i64",
		L"u64",
		L"i128",
		L"u128",
		L"isize",
		L"usize",

		L"f32",
		L"f64",

		L"size_of",
		L"String",

		// Traits
		L"Box",
		L"Rc",
		L"Arc",
		L"Pin",
		L"Deref",
		L"DerefMut",
		L"Drop",
		L"Copy",
		L"Clone",
		L"Send",
		L"Sync",
		L"Unpin",
		L"UnwindSafe",
		L"Sized"
	};
	static fStatHash_t map = { 0 };
	
	if ((lasti - start) < 1)
	{
		return;
	}

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_RUST_TOKEN_MEM, keyWords, ARRAYSIZE(keyWords));

	wchar kwBuf[MAX_KWBUF];
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

	for (u32 i = 0; i < (MAX_RUST_TOKEN_WORD - 1); ++i)
	{
		const u32 n = MAX_RUST_TOKEN_WORD - i;
		if ((n > filled) || ((filled - n) > 1))
		{
			continue;
		}
		else if (fStatHash_get(&map, kwBuf + (MAX_KWBUF - 1) - n))
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
void fStx_checkGoToken(fLine_t * restrict node, u32 start, u32 lasti, WORD kwCol)
{
	static usize memory[MAX_GO_TOKEN_MEM];

	// Rust keywords
	static const wchar * keyWords[] = {
		L"break",
		L"case",
		L"chan",
		L"const",
		L"continue",
		L"default",
		L"defere",
		L"else",
		L"fallthrough",
		L"for",
		L"func",
		L"go",
		L"goto",
		L"if",
		L"import",
		L"interface",
		L"map",
		L"package",
		L"range",
		L"return",
		L"select",
		L"struct",
		L"switch",
		L"type",
		L"var",

		// Built-in types
		L"string",
		L"bool",
		L"int8",
		L"uint8",
		L"byte",
		L"int16",
		L"uint16",
		L"int32",
		L"rune",
		L"uint32",
		L"int64",
		L"uint64",
		L"int",
		L"uint",
		L"uintptr",
		L"float32",
		L"float64",
		L"complex64",
		L"complex128"
	};
	static fStatHash_t map = { 0 };
	
	if ((lasti - start) < 1)
	{
		return;
	}

	fStatHash_initData(&map, memory, sizeof(usize) * MAX_GO_TOKEN_MEM, keyWords, ARRAYSIZE(keyWords));

	wchar kwBuf[MAX_KWBUF];
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

	for (u32 i = 0; i < (MAX_GO_TOKEN_WORD - 1); ++i)
	{
		const u32 n = MAX_GO_TOKEN_WORD - i;
		if ((n > filled) || ((filled - n) > 1))
		{
			continue;
		}
		else if (fStatHash_get(&map, kwBuf + (MAX_KWBUF - 1) - n))
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


#ifndef FKEYWORDS_CPP_H
#define FKEYWORDS_CPP_H

#include "../fCommon.h"

static const wchar * s_keyWordsCPP[] = {
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

#endif

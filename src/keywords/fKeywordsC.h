#ifndef FKEYWORDS_C_H
#define FKEYWORDS_C_H

#include "../fCommon.h"

static const wchar * s_keyWordsC[] = {
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

#endif
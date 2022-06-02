#ifndef FEMTO_COMMON_H
#define FEMTO_COMMON_H

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define NOMINMAX

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <math.h>

#include <safec.h>

#include "fProfiler.h"
#include "fErr.h"

// Conscise type definitions

typedef int8_t i8;
typedef uint8_t u8;
typedef int16_t i16;
typedef uint16_t u16;
typedef int32_t i32;
typedef uint32_t u32;
typedef int64_t i64;
typedef uint64_t u64;

typedef size_t usize;
typedef ssize_t isize;
typedef intptr_t iptr;
typedef uintptr_t uptr;
typedef void * vptr;

typedef float f32;
typedef double f64;
typedef long double f128;

typedef wchar_t wchar;
typedef unsigned char uchar;


// Since 0x88 - 0x8F are unassigned, they are free to be used by my editor
#define FEMTO_PROFILER_FILE "profiler.txt"
#define FEMTO_PROFILER_STACK_SIZE 256



#define FEMTO_SETTINGS_ERR_MAX 256

#define FEMTO_SETTINGS_FILE1 L"femto.json"
#define FEMTO_SETTINGS_FILE2 L"femto-settings.json"
#define FEMTO_SETTINGS_FILE3 L"femto_settings.json"
#define FEMTO_SETTINGS_FILE4 L"settings.json"

#define FEMTO_SETTINGS_MINTAB 1
#define FEMTO_SETTINGS_MAXTAB 32

#define FEMTO_SETTINGS_MINCOLOR 0
#define FEMTO_SETTINGS_MAXCOLOR UINT8_MAX


#define FEMTO_UNTITLED_NAME L"untitled"

#define MAX_STATUS 256

#define FEMTO_SHIFT_DEL 0x88

#define FEMTO_MOVELINE_UP   0x89
#define FEMTO_MOVELINE_DOWN 0x8A

#define FEMTO_DEFAULT_COLOR (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)

#define FEMTO_VERSION_STR "v2.1-pre"
#define FEMTO_VERSION_SEQ 2,1,0,0

#define fRGB(r, g, b) ( (COLORREF)( (DWORD)((BYTE)r) | (DWORD)(((BYTE)g) << 8) | (DWORD)(((BYTE)b) << 16) ) )

#endif

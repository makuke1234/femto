#ifndef COMMON_H
#define COMMON_H

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

#include "femtoProfiler.h"
#include "safec.h"

// Since 0x88 - 0x8F are unassigned, they are free to be used by my editor
#define FEMTO_SHIFT_DEL 0x88

#define FEMTO_MOVELINE_UP   0x89
#define FEMTO_MOVELINE_DOWN 0x8A

#define FEMTO_DEFAULT_COLOR (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY)

#endif

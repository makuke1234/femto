#include "fData.h"
#include "femto.h"


bool femtoData_reset(femtoData_t * restrict self)
{
	assert(self != NULL);
	*self = (femtoData_t){
		.prevConsoleMode    = 0,
		.bPrevConsoleModeSet = false,
		.conIn  = INVALID_HANDLE_VALUE,
		.conOut = INVALID_HANDLE_VALUE,
		.scrbuf = {
			.handle = INVALID_HANDLE_VALUE,
			.mem    = NULL,
			.w      = 0,
			.h      = 0
		},
		.filesSize = 0,
		.filesMax  = 1,
		.files     = malloc(sizeof(femtoFile_t *)),
		.cursorpos = malloc(sizeof(COORD)),
		.fileIdx   = -1
	};

	if ((self->files == NULL) || (self->cursorpos == NULL))
	{
		free(self->files);
		free(self->cursorpos);
		return false;
	}

	self->cursorpos[0] = (COORD){ 0, 0 };

	femtoSettings_reset(&self->settings);

	return true;
}
bool femtoData_init(femtoData_t * restrict self)
{
	assert(self != NULL);
	self->conIn  = GetStdHandle(STD_INPUT_HANDLE);
	self->conOut = GetStdHandle(STD_OUTPUT_HANDLE);
	// Set exit handler
	atexit(&femto_exitHandler);

	if (!GetConsoleMode(self->conIn, &self->prevConsoleMode))
	{
		return false;
	}

	// Set extended mode and disable quick edit mode
	if (!SetConsoleMode(self->conIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
	{
		return false;
	}
	self->bPrevConsoleModeSet = true;

	// Get console current size
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(self->conOut, &csbi))
	{
		return false;
	}

	self->scrbuf.w = (u32)(csbi.srWindow.Right  - csbi.srWindow.Left + 1);
	self->scrbuf.h = (u32)(csbi.srWindow.Bottom - csbi.srWindow.Top  + 1);

	writeProfiler("Screen buffer size: %u %u\n", self->scrbuf.w, self->scrbuf.h);
	// Create screen buffer
	self->scrbuf.handle = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL
	);
	if (self->scrbuf.handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	self->scrbuf.mem = malloc((usize)(self->scrbuf.w * self->scrbuf.h) * sizeof(CHAR_INFO));
	if (self->scrbuf.mem == NULL)
	{
		return false;
	}

	for (u32 i = 0, sz = self->scrbuf.w * self->scrbuf.h; i < sz; ++i)
	{
		self->scrbuf.mem[i] = (CHAR_INFO){
			.Char       = { .UnicodeChar = L' ' },
			.Attributes = FEMTO_DEFAULT_COLOR
		};
	}
	if (!SetConsoleScreenBufferSize(self->scrbuf.handle, (COORD){ .X = (SHORT)self->scrbuf.w, .Y = (SHORT)self->scrbuf.h }))
	{
		return false;
	}
	if (!SetConsoleActiveScreenBuffer(self->scrbuf.handle))
	{
		return false;
	}

	if (!femtoData_loadPalette(self))
	{
		return false;
	}

	return true;
}
bool femtoData_loadPalette(femtoData_t * restrict self)
{
	assert(self != NULL);

	femtoPalette_t * pal = &self->settings.palette;
	if (!pal->bUsePalette)
	{
		return true;
	}

	// Try to apply new palette
	CONSOLE_SCREEN_BUFFER_INFOEX csbiex;
	csbiex.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	if (!GetConsoleScreenBufferInfoEx(self->scrbuf.handle, &csbiex))
	{
		return false;
	}

	// Copy palette
	for (uint8_t i = 0; i < MAX_COLORS; ++i)
	{
		const COLORREF old = csbiex.ColorTable[i];
		femtoColor_t * oldCol = &self->settings.palette.oldColors[i];
		oldCol->r = GetRValue(old);
		oldCol->g = GetGValue(old);
		oldCol->b = GetBValue(old);

		const femtoColor_t col = self->settings.palette.colors[i];
		csbiex.ColorTable[i] = fRGB(col.r, col.g, col.b);
	}

	// Apply new
	++csbiex.srWindow.Right;
	++csbiex.srWindow.Bottom;
	if (!SetConsoleScreenBufferInfoEx(self->scrbuf.handle, &csbiex))
	{
		return false;
	}

	return true;
}
bool femtoData_restorePalette(const femtoData_t * restrict self)
{
	assert(self != NULL);

	const femtoPalette_t * pal = &self->settings.palette;
	if (!pal->bUsePalette)
	{
		return true;
	}

	// Restore old palette

	CONSOLE_SCREEN_BUFFER_INFOEX csbiex;
	csbiex.cbSize = sizeof(CONSOLE_SCREEN_BUFFER_INFOEX);
	if (!GetConsoleScreenBufferInfoEx(self->scrbuf.handle, &csbiex))
	{
		return false;
	}

	for (uint8_t i = 0; i < MAX_COLORS; ++i)
	{
		const femtoColor_t col = self->settings.palette.oldColors[i];
		csbiex.ColorTable[i] = fRGB(col.r, col.g, col.b);
	}

	++csbiex.srWindow.Right;
	++csbiex.srWindow.Bottom;
	if (!SetConsoleScreenBufferInfoEx(self->scrbuf.handle, &csbiex))
	{
		return false;
	}

	return true;
}

void femtoData_refresh(femtoData_t * restrict self)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	u32 line;
	if (femto_updateScrbuf(self, &line) == false)
	{
		WriteConsoleOutputW(
			self->scrbuf.handle,
			self->scrbuf.mem,
			(COORD){ .X = (SHORT)self->scrbuf.w, .Y = (SHORT)self->scrbuf.h },
			(COORD){ 0, 0 },
			&(SMALL_RECT){ .Left = 0, .Top = 0, .Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)(self->scrbuf.h - 2) }
		);
	}
	else
	{
		// Only refresh 1 line
		WriteConsoleOutputW(
			self->scrbuf.handle,
			self->scrbuf.mem,
			(COORD){ .X = (SHORT)self->scrbuf.w, .Y = (SHORT)self->scrbuf.h },
			(COORD){ .X = 0, .Y = (SHORT)line },
			&(SMALL_RECT){ .Left = 0, .Top = (SHORT)line, .Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)line }
		);
	}
}
void femtoData_refreshThread(femtoData_t * restrict self)
{
	assert(self != NULL);
	femtoDrawThread_t * dt = &self->drawThread;

	EnterCriticalSection(&dt->crit);

	dt->bReady = true;
	WakeConditionVariable(&dt->cv);

	LeaveCriticalSection(&dt->crit);
}
void femtoData_refreshAll(femtoData_t * restrict self)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	u32 line;	// Phantom variable
	femto_updateScrbuf(self, &line);
	WriteConsoleOutputW(
		self->scrbuf.handle,
		self->scrbuf.mem,
		(COORD){ .X = (SHORT)self->scrbuf.w, .Y = (SHORT)self->scrbuf.h },
		(COORD){ 0, 0 },
		&(SMALL_RECT){ .Left = 0, .Top = 0, .Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)(self->scrbuf.h - 1) }
	);
}
void femtoData_statusDraw(femtoData_t * restrict self, const wchar * restrict message, const WORD * restrict colorData)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	assert(message != NULL);

	u32 effLen = min_u32((u32)wcslen(message), self->scrbuf.w);
	CHAR_INFO * restrict lastLine = self->scrbuf.mem + (self->scrbuf.h - 1) * self->scrbuf.w;
	for (usize i = 0; i < effLen; ++i)
	{
		lastLine[i] = (CHAR_INFO){
			.Char       = { .UnicodeChar = message[i] },
			.Attributes = FEMTO_DEFAULT_COLOR
		};
	}
	if (colorData != NULL)
	{
		for (usize i = 0; i < effLen; ++i)
		{
			lastLine[i].Attributes = colorData[i];
		}
	}
	for (usize i = effLen; i < self->scrbuf.w; ++i)
	{
		lastLine[i] = (CHAR_INFO){
			.Char       = { .UnicodeChar = L' ' },
			.Attributes = FEMTO_DEFAULT_COLOR
		};
	}
	femtoData_statusRefresh(self);
}
void femtoData_statusRefresh(femtoData_t * restrict self)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	WriteConsoleOutputW(
		self->scrbuf.handle,
		self->scrbuf.mem,
		(COORD){ .X = (SHORT)self->scrbuf.w, .Y = (SHORT)self->scrbuf.h },
		(COORD){ .X = 0, .Y = (SHORT)(self->scrbuf.h - 1) },
		&(SMALL_RECT){ .Left = 0, .Top = (SHORT)(self->scrbuf.h - 1), .Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)(self->scrbuf.h - 1) }
	);
}

bool femtoData_openTab(femtoData_t * restrict self, const wchar * restrict fileName)
{
	assert(self != NULL);
	assert(fileName != NULL);

	// Reallocate tabs
	if (self->filesSize >= self->filesMax)
	{
		const u32 newcap = (self->filesSize + 1) * 2;
		vptr mem = realloc(self->files, sizeof(femtoFile_t *) * newcap);
		if (mem == NULL)
		{
			return false;
		}

		self->files    = mem;

		mem = realloc(self->cursorpos, sizeof(COORD) * newcap);
		if (mem == NULL)
		{
			return false;
		}

		self->cursorpos = mem;

		self->filesMax = newcap;
	}
	
	if ((self->files[self->filesSize] = femtoFile_resetDyn()) == NULL)
	{
		return false;
	}
	if (!femtoFile_open(self->files[self->filesSize], fileName, false))
	{
		return false;
	}

	self->fileIdx = (i32)self->filesSize;
	++self->filesSize;

	self->cursorpos[self->fileIdx] = (COORD) { 0, 0 };

	femtoFile_close(self->files[self->fileIdx]);

	// Set console title
	femtoFile_setConTitle(self->files[self->fileIdx]);

	return true;
}
void femtoData_closeTab(femtoData_t * restrict self)
{
	assert(self != NULL);
	assert(self->fileIdx != -1);

	femtoFile_t * file = self->files[self->fileIdx];
	
	// Remove file from tab list
	--self->filesSize;
	for (u32 i = (u32)self->fileIdx; i < self->filesSize; ++i)
	{
		self->files[i]     = self->files[i + 1];
		self->cursorpos[i] = self->cursorpos[i + 1];
	}

	// Select next tab to be active if possible, otherwise select previous, if only 1 is open, set to NULL
	if ((self->fileIdx > 0) && (self->filesSize > 0))
	{
		--self->fileIdx;
	}
	else if (!((u32)self->fileIdx < self->filesSize))
	{
		self->fileIdx = -1;
	}

	// Delete "file"
	femtoFile_destroy(file);
	free(file);

	// Set console title back
	if (self->fileIdx != -1)
	{
		femtoFile_setConTitle(self->files[self->fileIdx]);
	}
}


void femtoData_destroy(femtoData_t * restrict self)
{
	assert(self != NULL);
	if (self->scrbuf.mem != NULL)
	{
		free(self->scrbuf.mem);
		self->scrbuf.mem = NULL;
	}
	if (self->scrbuf.handle != INVALID_HANDLE_VALUE)
	{
		SetConsoleActiveScreenBuffer(self->conOut);
	}

	if (self->bPrevConsoleModeSet)
	{
		self->bPrevConsoleModeSet = false;
		SetConsoleMode(self->conIn, self->prevConsoleMode);
	}

	self->fileIdx = -1;
	for (usize i = 0; i < self->filesSize; ++i)
	{
		femtoFile_destroy(self->files[i]);
		free(self->files[i]);
	}
	free(self->files);
	self->files     = NULL;
	self->filesSize = 0;
	self->filesMax  = 0;

	femtoData_restorePalette(self);

	femtoSettings_destroy(&self->settings);
}

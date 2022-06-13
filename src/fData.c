#include "fData.h"
#include "femto.h"


bool fData_reset(fData_t * restrict self)
{
	assert(self != NULL);

	*self = (fData_t){
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
		.filesSize   = 0,
		.filesMax    = 1,
		.files       = malloc(sizeof(fFile_t *)),
		.cursorpos   = malloc(sizeof(COORD)),
		.fileIdx     = -1,
		.searchBuf   = { [0] = L'\0' },
		.psearchTerm = NULL,
		.bDirBack    = false
	};

	if ((self->files == NULL) || (self->cursorpos == NULL))
	{
		free(self->files);
		free(self->cursorpos);
		return false;
	}

	self->cursorpos[0] = (COORD){ 0, 0 };

	fSettings_reset(&self->settings);

	return true;
}
bool fData_init(fData_t * restrict self)
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

	fProf_write("Screen buffer size: %u %u", self->scrbuf.w, self->scrbuf.h);
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

	self->scrbuf.mem = malloc((usize)self->scrbuf.w * (usize)self->scrbuf.h * sizeof(CHAR_INFO));
	if (self->scrbuf.mem == NULL)
	{
		return false;
	}

	for (usize i = 0, sz = (usize)self->scrbuf.w * (usize)self->scrbuf.h; i < sz; ++i)
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

	if (!fData_loadPalette(self))
	{
		return false;
	}

	return true;
}
bool fData_loadPalette(fData_t * restrict self)
{
	assert(self != NULL);

	fPalette_t * restrict pal = &self->settings.palette;
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
	for (u8 i = 0; i < MAX_CONSOLE_COLORS; ++i)
	{
		const COLORREF old = csbiex.ColorTable[i];
		fColor_t * restrict oldCol = &self->settings.palette.oldColors[i];
		oldCol->r = GetRValue(old);
		oldCol->g = GetGValue(old);
		oldCol->b = GetBValue(old);

		const fColor_t col = self->settings.palette.colors[i];
		csbiex.ColorTable[i] = fRGB(col.r, col.g, col.b);
	}

	// Apply new
	++csbiex.srWindow.Right;
	++csbiex.srWindow.Bottom;
	return SetConsoleScreenBufferInfoEx(self->scrbuf.handle, &csbiex) != 0;
}
bool fData_restorePalette(const fData_t * restrict self)
{
	assert(self != NULL);

	fProf_write("Restoring palette...");

	const fPalette_t * restrict pal = &self->settings.palette;
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

	for (u8 i = 0; i < MAX_CONSOLE_COLORS; ++i)
	{
		const fColor_t col = self->settings.palette.oldColors[i];
		csbiex.ColorTable[i] = fRGB(col.r, col.g, col.b);
	}

	++csbiex.srWindow.Right;
	++csbiex.srWindow.Bottom;

	fProf_write("Almost complete...");
	return SetConsoleScreenBufferInfoEx(self->scrbuf.handle, &csbiex) != 0;
}

void fData_refreshEdit(fData_t * restrict self)
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
			&(SMALL_RECT){
				.Left = 0, .Top = 0,
				.Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)(self->scrbuf.h - 2)
			}
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
			&(SMALL_RECT){
				.Left = 0, .Top = (SHORT)line,
				.Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)line
			}
		);
	}
}
void fData_refreshEditAsync(fData_t * restrict self)
{
	assert(self != NULL);

	fDrawThreadData_t * dt = &self->drawThread;

	EnterCriticalSection(&dt->crit);

	dt->bReady = true;
	WakeConditionVariable(&dt->cv);

	LeaveCriticalSection(&dt->crit);
}
void fData_refreshAll(fData_t * restrict self)
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
		&(SMALL_RECT){
			.Left = 0, .Top = 0,
			.Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)(self->scrbuf.h - 1)
		}
	);
}
void fData_statusMsg(fData_t * restrict self, const wchar * restrict message, const WORD * restrict colorData)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	assert(message != NULL);

	const usize effLen = min_usize(wcslen(message), (usize)self->scrbuf.w);
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
	fData_statusRefresh(self);
}
void fData_statusRefresh(fData_t * restrict self)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	
	WriteConsoleOutputW(
		self->scrbuf.handle,
		self->scrbuf.mem,
		(COORD){ .X = (SHORT)self->scrbuf.w, .Y = (SHORT)self->scrbuf.h },
		(COORD){ .X = 0, .Y = (SHORT)(self->scrbuf.h - 1) },
		&(SMALL_RECT){
			.Left = 0, .Top = (SHORT)(self->scrbuf.h - 1),
			.Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)(self->scrbuf.h - 1)
		}
	);
}

void fData_cancelSearch(fData_t * restrict self)
{
	assert(self != NULL);

	fFile_t * restrict pfile = self->files[self->fileIdx];
	assert(pfile != NULL);

	self->psearchTerm = NULL;
	pfile->data.bUpdateAll = true;
	fData_refreshEdit(self);
}
void fData_cancelHighlight(fData_t * restrict self)
{
	assert(self != NULL);

	fFile_t * restrict pfile = self->files[self->fileIdx];
	assert(pfile != NULL);

	pfile->data.hl.beg = NULL;
	pfile->data.bUpdateAll = true;
	fData_refreshEditAsync(self);
}

bool fData_openTab(fData_t * restrict self, const wchar * restrict fileName)
{
	assert(self != NULL);

	// Reallocate tabs
	if (self->filesSize >= self->filesMax)
	{
		const usize newcap = (self->filesSize + 1) * 2;
		vptr mem = realloc(self->files, sizeof(fFile_t *) * newcap);
		if (mem == NULL)
		{
			return false;
		}

		self->files = mem;

		mem = realloc(self->cursorpos, sizeof(COORD) * newcap);
		if (mem == NULL)
		{
			return false;
		}

		self->cursorpos = mem;

		self->filesMax = newcap;
	}
	
	if ((self->files[self->filesSize] = fFile_resetDyn()) == NULL)
	{
		return false;
	}
	if (!fFile_open(self->files[self->filesSize], fileName, false))
	{
		return false;
	}

	self->fileIdx = (isize)self->filesSize;
	++self->filesSize;

	self->cursorpos[self->fileIdx] = (COORD){ 0, 0 };

	fFile_close(self->files[self->fileIdx]);

	// Set console title
	femto_setConTitle(self->files[self->fileIdx]->fileName);

	return true;
}
void fData_closeTab(fData_t * restrict self)
{
	assert(self != NULL);
	assert(self->fileIdx != -1);

	fFile_t * restrict file = self->files[self->fileIdx];
	
	// Remove file from tab list
	--self->filesSize;
	for (usize i = (usize)self->fileIdx; i < self->filesSize; ++i)
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
	fFile_free(file);

	// Set console title back
	if (self->fileIdx != -1)
	{
		femto_setConTitle(self->files[self->fileIdx]->fileName);
	}
}


void fData_destroy(fData_t * restrict self)
{
	assert(self != NULL);

	if (self->scrbuf.mem != NULL)
	{
		free(self->scrbuf.mem);
		self->scrbuf.mem = NULL;
	}
	if (self->scrbuf.handle != INVALID_HANDLE_VALUE)
	{
		fData_restorePalette(self);
		SetConsoleActiveScreenBuffer(self->conOut);
	}

	if (self->bPrevConsoleModeSet)
	{
		self->bPrevConsoleModeSet = false;
		SetConsoleMode(self->conIn, self->prevConsoleMode);
	}

	self->fileIdx = -1;
	for (u32 i = 0; i < self->filesSize; ++i)
	{
		fFile_free(self->files[i]);
	}
	free(self->files);
	self->files     = NULL;
	self->filesSize = 0;
	self->filesMax  = 0;

	fSettings_destroy(&self->settings);
}

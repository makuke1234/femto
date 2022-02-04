#include "femtoData.h"
#include "femto.h"


bool femtoData_reset(femtoData_t * restrict self)
{
	assert(self != NULL);
	*self = (femtoData_t){
		.prevConsoleMode    = 0,
		.prevConsoleModeSet = false,
		.conIn  = INVALID_HANDLE_VALUE,
		.conOut = INVALID_HANDLE_VALUE,
		.scrbuf = {
			.handle = INVALID_HANDLE_VALUE,
			.mem    = NULL,
			.w      = 0,
			.h      = 0
		},
		.cursorpos = { 0, 0 },
		.filesSize = 1,
		.filesMax  = 1,
		.files     = malloc(sizeof(femtoFile_t *)),
		.file      = NULL
	};

	if (self->files == NULL)
	{
		return false;
	}
	// Allocate memory for 1 file
	if ((self->files[0] = femtoFile_resetDyn()) == NULL)
	{
		return false;
	};
	self->file = self->files[0];

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

	// Set extended mode
	if (!SetConsoleMode(self->conIn, ENABLE_EXTENDED_FLAGS))
	{
		return false;
	}
	// Disable quick edit mode
	if (!SetConsoleMode(self->conIn, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT))
	{
		return false;
	}
	self->prevConsoleModeSet = true;

	// Get console current size
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(self->conOut, &csbi))
	{
		return false;
	}

	self->scrbuf.w = (uint32_t)(csbi.srWindow.Right  - csbi.srWindow.Left + 1);
	self->scrbuf.h = (uint32_t)(csbi.srWindow.Bottom - csbi.srWindow.Top  + 1);

	writeProfiler("femtoData_init", "Screen buffer size: %u %u\n", self->scrbuf.w, self->scrbuf.h);
	// Create screen buffer
	self->scrbuf.handle = CreateConsoleScreenBuffer(
		GENERIC_WRITE,
		0,
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL
	);
	if (self->scrbuf.handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	self->scrbuf.mem = malloc((size_t)(self->scrbuf.w * self->scrbuf.h) * sizeof(CHAR_INFO));
	if (self->scrbuf.mem == NULL)
	{
		return false;
	}

	for (uint32_t i = 0, sz = self->scrbuf.w * self->scrbuf.h; i < sz; ++i)
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

	return true;
}
void femtoData_refresh(femtoData_t * restrict self)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	uint32_t line;
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
void femtoData_refreshAll(femtoData_t * restrict self)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	uint32_t line;	// Phantom variable
	femto_updateScrbuf(self, &line);
	WriteConsoleOutputW(
		self->scrbuf.handle,
		self->scrbuf.mem,
		(COORD){ .X = (SHORT)self->scrbuf.w, .Y = (SHORT)self->scrbuf.h },
		(COORD){ 0, 0 },
		&(SMALL_RECT){ .Left = 0, .Top = 0, .Right = (SHORT)(self->scrbuf.w - 1), .Bottom = (SHORT)(self->scrbuf.h - 1) }
	);
}
void femtoData_statusDraw(femtoData_t * restrict self, const wchar_t * restrict message, const WORD * restrict colorData)
{
	assert(self != NULL);
	assert(self->scrbuf.mem != NULL);
	assert(message != NULL);

	uint32_t effLen = u32Min((uint32_t)wcslen(message), self->scrbuf.w);
	CHAR_INFO * restrict lastLine = self->scrbuf.mem + (self->scrbuf.h - 1) * self->scrbuf.w;
	for (size_t i = 0; i < effLen; ++i)
	{
		lastLine[i] = (CHAR_INFO){
			.Char       = { .UnicodeChar = message[i] },
			.Attributes = FEMTO_DEFAULT_COLOR
		};
	}
	if (colorData != NULL)
	{
		for (size_t i = 0; i < effLen; ++i)
		{
			lastLine[i].Attributes = colorData[i];
		}
	}
	for (size_t i = effLen; i < self->scrbuf.w; ++i)
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

	if (self->prevConsoleModeSet)
	{
		self->prevConsoleModeSet = false;
		SetConsoleMode(self->conIn, self->prevConsoleMode);
	}

	self->file = NULL;
	for (size_t i = 0; i < self->filesSize; ++i)
	{
		femtoFile_destroy(self->files[i]);
		free(self->files[i]);
	}
	free(self->files);
	self->files     = NULL;
	self->filesSize = 0;
	self->filesMax  = 0;
}

#include "femtoData.h"
#include "femto.h"


void femtoData_reset(femtoData_t * restrict self)
{
	*self = (femtoData_t){
		.conIn  = INVALID_HANDLE_VALUE,
		.conOut = INVALID_HANDLE_VALUE,
		.scrbuf = {
			.handle = INVALID_HANDLE_VALUE,
			.mem    = NULL,
			.w      = 0,
			.h      = 0
		},
		.cursorpos = { 0, 0 }
	};
	femtoFile_reset(&self->file);
}
bool femtoData_init(femtoData_t * restrict self)
{
	self->conIn  = GetStdHandle(STD_INPUT_HANDLE);
	self->conOut = GetStdHandle(STD_OUTPUT_HANDLE);
	// Set exit handler
	atexit(&femto_exitHandler);

	// Get console current size
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (!GetConsoleScreenBufferInfo(self->conOut, &csbi))
	{
		return false;
	}

	self->scrbuf.w = (uint32_t)(csbi.srWindow.Right  - csbi.srWindow.Left + 1);
	self->scrbuf.h = (uint32_t)(csbi.srWindow.Bottom - csbi.srWindow.Top  + 1);
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

	self->scrbuf.mem = malloc((size_t)(self->scrbuf.w * self->scrbuf.h) * sizeof(wchar_t));
	if (self->scrbuf.mem == NULL)
	{
		return false;
	}

	for (uint32_t i = 0, sz = self->scrbuf.w * self->scrbuf.h; i < sz; ++i)
	{
		self->scrbuf.mem[i] = L' ';
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
	femto_updateScrbuf(self);
	DWORD dwBytes;
	WriteConsoleOutputCharacterW(
		self->scrbuf.handle,
		self->scrbuf.mem,
		self->scrbuf.w * (self->scrbuf.h - 1),
		(COORD){ 0, 0 },
		&dwBytes
	);
}
void femtoData_refreshAll(femtoData_t * restrict self)
{
	femto_updateScrbuf(self);
	DWORD dwBytes;
	WriteConsoleOutputCharacterW(
		self->scrbuf.handle,
		self->scrbuf.mem,
		self->scrbuf.w * self->scrbuf.h,
		(COORD){ 0, 0 },
		&dwBytes
	);
}
void femtoData_statusDraw(femtoData_t * restrict self, const wchar_t * message)
{
	uint32_t effLen = u32Min((uint32_t)wcslen(message), self->scrbuf.w);
	wchar_t * restrict lastLine = self->scrbuf.mem + (self->scrbuf.h - 1) * self->scrbuf.w;
	memcpy(
		lastLine,
		message,
		sizeof(wchar_t) * effLen
	);
	for (size_t i = effLen; i < self->scrbuf.w; ++i)
	{
		lastLine[i] = L' ';
	}
	femtoData_statusRefresh(self);
}
void femtoData_statusRefresh(femtoData_t * restrict self)
{
	DWORD dwBytes;
	WriteConsoleOutputCharacterW(
		self->scrbuf.handle,
		self->scrbuf.mem + (self->scrbuf.h - 1) * self->scrbuf.w,
		self->scrbuf.w,
		(COORD){ .X = 0, .Y = (SHORT)(self->scrbuf.h - 1) },
		&dwBytes
	);
}

void femtoData_destroy(femtoData_t * restrict self)
{
	if (self->scrbuf.mem != NULL)
	{
		free(self->scrbuf.mem);
		self->scrbuf.mem = NULL;
	}
	if (self->scrbuf.handle != INVALID_HANDLE_VALUE)
	{
		SetConsoleActiveScreenBuffer(self->conOut);
	}
	femtoFile_destroy(&self->file);
}

#include "femto.h"
#include "fData.h"


i32 min_i32(i32 a, i32 b)
{
	return (a < b) ? a : b;
}
i32 max_i32(i32 a, i32 b)
{
	return (a < b) ? b : a;
}
i32 clamp_i32(i32 value, i32 min, i32 max)
{
	return (value < min) ? min : (value > max) ? max : value;
}

u32 min_u32(u32 a, u32 b)
{
	return (a < b) ? a : b;
}
u32 max_u32(u32 a, u32 b)
{
	return (a < b) ? b : a;
}
u32 clamp_u32(u32 value, u32 min, u32 max)
{
	return (value < min) ? min : (value > max) ? max : value;
}

i64 min_i64(i64 a, i64 b)
{
	return (a < b) ? a : b;
}
i64 max_i64(i64 a, i64 b)
{
	return (a < b) ? b : a;
}
i64 clamp_i64(i64 value, i64 min, i64 max)
{
	return (value < min) ? min : (value > max) ? max : value;
}

u64 min_u64(u64 a, u64 b)
{
	return (a < b) ? a : b;
}
u64 max_u64(u64 a, u64 b)
{
	return (a < b) ? b : a;
}
u64 clamp_u64(u64 value, u64 min, u64 max)
{
	return (value < min) ? min : (value > max) ? max : value;
}


usize min_usize(usize a, usize b)
{
	return (a < b) ? a : b;
}
usize max_usize(usize a, usize b)
{
	return (a < b) ? b : a;
}
usize clamp_usize(usize value, usize min, usize max)
{
	return (value < min) ? min : (value > max) ? max : value;
}


char * femto_cpcat_s(char ** restrict pstr, usize * restrict psize, usize * plen, wchar cp)
{
	uchar conv[3];
	usize len = 0;
	if (cp <= 0x7F)
	{
		len = 1;
		conv[0] = (uchar)cp;
	}
	else if (cp <= 0x07FF)
	{
		len = 2;
		conv[0] = (u8)(0xC0 | ((cp >> 6) & 0x1F));
		conv[1] = (u8)(0x80 | ( cp       & 0x3F));
	}
	else
	{
		len = 3;
		conv[0] = (u8)(0xE0 | ((cp >> 12) & 0x0F));
		conv[1] = (u8)(0x80 | ((cp >>  6) & 0x3F));
		conv[2] = (u8)(0x80 | ( cp        & 0x3F));
	}

	// Add converted string to original array
	char * ret = dynstrncat_s(pstr, psize, *plen, (char *)conv, len);
	*plen = (ret != NULL) ? (*plen + len) : *plen;
	return ret;
}


char * femto_escStr(const char * restrict inp)
{
	assert(inp != NULL);

	return femto_escStr_s(inp, strlen(inp));
}
char * femto_escStr_s(const char * restrict inp, usize len)
{
	assert(inp != NULL);
	assert(len > 0);

	char * mem = NULL;
	usize mlen = 0, mcap = 0;

	for (usize i = 0; i < len; ++i)
	{
		switch (inp[i])
		{
		case '\\':
			if ((i + 1) < len)
			{
				++i;
				char ch = '\0';
				switch (inp[i])
				{
				case 'b':
					ch = '\b';
					break;
				case 'f':
					ch = '\f';
					break;
				case 'r':
					ch = '\r';
					break;
				case 'n':
					ch = '\n';
					break;
				case 't':
					ch = '\t';
					break;
				case 'u':
					// 4-wide unicode code point
					if ((i + 4) < len)
					{
						++i;

						usize end = i;
						u16 value = 0;
						for (; ((end - i) <= 4) && (end < len); ++end)
						{
							char t = (char)tolower(inp[end]);
							if (!(((t >= '0') && (t <= '9')) || ((t >= 'a') && (t <= 'f'))))
							{
								break;
							}

							const u8 digit = (t >= 'A') ? (u8)(t - 'A' + 10) : (u8)(t - '0');
							value = (u16)(value * 16 + digit);
						}

						if (femto_cpcat_s(&mem, &mcap, &mlen, value) == NULL)
						{
							if (mem != NULL)
							{
								free(mem);
							}
							return NULL;
						}
						i = end - 1;
					}
					else
					{
						if (mem != NULL)
						{
							free(mem);
						}
						return NULL;
					}
					break;
				default:
					if ((inp[i] >= '0') && (inp[i] <= '7'))
					{
						// Maximum 3 chars
						u16 value = 0;
						usize end = i;
						for (; ((end - i) <= 3) && (end < len); ++end)
						{
							if (!((inp[end] >= '0') && (inp[end] <= '7')))
							{
								break;
							}
							u16 newvalue = (u16)(value * 8 + (inp[end] - '0'));
							if (newvalue > 127)
							{
								break;
							}
							value = newvalue;
						}

						ch = (char)value;
						i = end - 1;
					}
					else
					{
						ch = inp[i];
					}
				}
				
				if (ch != '\0')
				{
					if (dynstrncat_s(&mem, &mcap, mlen, &ch, 1) == NULL)
					{
						if (mem != NULL)
						{
							free(mem);
						}
						return NULL;
					}
					++mlen;
				}
			}
			else
			{
				if (mem != NULL)
				{
					free(mem);
				}
				return NULL;
			}
			break;
		default:
			if (dynstrncat_s(&mem, &mcap, mlen, &inp[i], 1) == NULL)
			{
				if (mem != NULL)
				{
					free(mem);
				}
				return NULL;
			}
			++mlen;
		}
	}

	return mem;
}


static fData_t * s_atExitData = NULL;

void femto_exitHandlerSetVars(fData_t * restrict pdata)
{
	assert(pdata != NULL);

	s_atExitData = pdata;
}
void femto_exitHandler(void)
{
	// Clear resources
	fData_destroy(s_atExitData);
}

void femto_printHelp(const wchar * restrict app)
{
	assert(app != NULL);

	fwprintf(stderr, L"Correct usage:\n%S [options] [file]\n", app);
}
void femto_printHelpClue(const wchar * restrict app)
{
	assert(app != NULL);

	fwprintf(stderr, L"To show all available commands type:\n%S --help\n", app);
}

bool femto_askInput(fData_t * restrict peditor, wchar * restrict line, u32 maxLen)
{
	assert(peditor != NULL);
	assert(line != NULL);
	assert(maxLen > 0);
	maxLen = min_u32(maxLen, peditor->scrbuf.w + 1);

	fSettings_t * restrict pset = &peditor->settings;

	fLine_t temp;
	fLine_init(&temp);

	// Find first non-space on the last line
	CHAR_INFO * restrict lastline = &peditor->scrbuf.mem[(usize)peditor->scrbuf.w * (usize)(peditor->scrbuf.h - 1)];

	u32 sidx = peditor->scrbuf.w - 1;
	for (; sidx > 0; --sidx)
	{
		if (lastline[sidx].Char.UnicodeChar != L' ')
		{
			++sidx;
			break;
		}
	}

	// Start asking input
	COORD cur = { .X = (SHORT)sidx, .Y = (SHORT)(peditor->scrbuf.h - 1) };
	SetConsoleCursorPosition(peditor->scrbuf.handle, cur);

	line[0] = L'\0';
	
	bool read = true, update = false, updateCur = false;
	while (1)
	{
		INPUT_RECORD ir;
		DWORD evRead;
		if (!ReadConsoleInputW(peditor->conIn, &ir, 1, &evRead) || !evRead)
		{
			continue;
		}

		if ((ir.EventType == KEY_EVENT) && ir.Event.KeyEvent.bKeyDown)
		{
			update    = false;
			updateCur = false;
			const wchar key      = ir.Event.KeyEvent.uChar.UnicodeChar;
			const wchar wVirtKey = ir.Event.KeyEvent.wVirtualKeyCode;

			if ((wVirtKey == VK_ESCAPE) || (wVirtKey == VK_RETURN))
			{
				read = (wVirtKey == VK_RETURN);
				break;
			}
			else if (key > sacLAST_CODE)
			{
				// If there's room in the array
				if ((temp.lineEndx - temp.freeSpaceLen + 1) < maxLen)
				{
					// Normal keys

					// add key
					if (!fLine_addChar(&temp, key, pset->tabWidth))
					{
						return false;
					}

					// reload contents to line
					update = true;
				}
			}
			else
			{
				// Special keys
				switch (wVirtKey)
				{
				case VK_DELETE:
					if ((temp.curx + temp.freeSpaceLen) < temp.lineEndx)
					{
						++temp.freeSpaceLen;
						update = true;
					}
					break;
				case VK_BACK:
					if (temp.curx > 0)
					{
						--temp.curx;
						++temp.freeSpaceLen;
						fLine_calcVirtCursor(&temp, pset->tabWidth);
						update = true;
					}
					break;
				case VK_LEFT:
					if (temp.curx > 0)
					{
						fLine_moveCursor(&temp, -1);
						fLine_calcVirtCursor(&temp, pset->tabWidth);
						updateCur = true;
					}
					break;
				case VK_RIGHT:
					if ((temp.curx + temp.freeSpaceLen) < temp.lineEndx)
					{
						fLine_moveCursor(&temp, 1);
						fLine_calcVirtCursor(&temp, pset->tabWidth);
						updateCur = true;
					}
					break;
				case VK_HOME:
					fLine_moveCursor(&temp, -(isize)temp.lineEndx);
					fLine_calcVirtCursor(&temp, pset->tabWidth);
					updateCur = true;
					break;
				case VK_END:
					fLine_moveCursor(&temp, (isize)temp.lineEndx);
					fLine_calcVirtCursor(&temp, pset->tabWidth);
					updateCur = true;
					break;
				}
			}

			// Update last line
			if (update)
			{
				fLine_getTextLim(&temp, line, (usize)maxLen);
				fData_statusMsg(peditor, line, NULL);
			}
			if (update | updateCur)
			{
				// Update cursor position
				cur.X = (SHORT)temp.virtcurx;
				SetConsoleCursorPosition(peditor->scrbuf.handle, cur);
			}
		}
	}

	fLine_destroy(&temp);

	SetConsoleCursorPosition(peditor->scrbuf.handle, peditor->cursorpos[peditor->fileIdx]);
	return read;
}

static inline bool s_femto_inner_quit(fData_t * restrict peditor, wchar * restrict tempstr, wchar key, const wchar * restrict normMsg)
{
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		return false;
	}
	u32 realLen = 0;

	realLen += (u32)swprintf_s(tempstr, MAX_STATUS, L"Unsaved file(s): ");

	// Scan for any unsaved work
	bool unsavedAny = false;
	for (usize i = 0; i < peditor->filesSize; ++i)
	{
		fFile_t * restrict f = peditor->files[i];
		fFile_checkUnsaved(f, NULL, NULL);
		if (f->bUnsaved && (realLen < MAX_STATUS))
		{
			unsavedAny = true;
			realLen += (u32)swprintf_s(tempstr + realLen, MAX_STATUS - realLen, L"%s; ", f->fileName);
		}
	}
	if (!unsavedAny)
	{
		return false;
	}
	else if (realLen < MAX_STATUS)
	{
		swprintf_s(tempstr + realLen, MAX_STATUS - realLen, L"Press %s to confirm exit", (key == sacCTRL_Q) ? L"Ctrl+Shift+Q" : normMsg);
	}

	return true;
}
static inline void s_femto_inner_openTab(fData_t * restrict peditor, wchar * restrict tempstr, const wchar * restrict inp)
{
	const wchar * restrict res = NULL;
	const isize oldIdx = peditor->fileIdx;
	if (fData_openTab(peditor, inp) && ((res = fFile_read(peditor->files[peditor->fileIdx])) == NULL) )
	{
		swprintf_s(
			tempstr, MAX_STATUS,
			L"Opened %s successfully; %s%s EOL sequences; Syntax: %S",
			(inp == NULL) ? L"new tab" : inp,
			(peditor->files[peditor->fileIdx]->eolSeq & eolCR) ? L"CR" : L"",
			(peditor->files[peditor->fileIdx]->eolSeq & eolLF) ? L"LF" : L"",
			fStx_name(peditor->files[peditor->fileIdx]->syntax)
		);
		fData_refreshEdit(peditor);
	}
	else if (res != NULL)
	{
		wcscpy_s(tempstr, MAX_STATUS, res);
		fData_closeTab(peditor);
		peditor->fileIdx = oldIdx;
	}
	else
	{
		swprintf_s(tempstr, MAX_STATUS, L"Failure while opening %s!", (inp == NULL) ? L"new tab" : inp);
	}
}
static inline void s_femto_inner_closeTab(fData_t * restrict peditor, wchar * restrict tempstr, bool forceClose)
{
	assert(peditor != NULL);
	assert(tempstr != NULL);

	if (!forceClose)
	{
		u32 realLen = 0;

		realLen += (u32)swprintf_s(tempstr, MAX_STATUS, L"File ");

		// Scan for any unsaved work
		fFile_checkUnsaved(peditor->files[peditor->fileIdx], NULL, NULL);
		if (peditor->files[peditor->fileIdx]->bUnsaved && (realLen < MAX_STATUS))
		{
			realLen += (u32)swprintf_s(tempstr + realLen, MAX_STATUS - realLen, L"%s is unsaved; ", peditor->files[peditor->fileIdx]->fileName);
			if (realLen < MAX_STATUS)
			{
				swprintf_s(tempstr + realLen, MAX_STATUS - realLen, L"Press %s to confirm closing", L"Ctrl+Shift+W");
			}
			return;
		}
	}

	const wchar * restrict fname = peditor->files[peditor->fileIdx]->fileName;
	swprintf_s(tempstr, MAX_STATUS, L"Closed tab %s", (fname == NULL) ? L"untitled" : fname);
	fData_closeTab(peditor);
	
	peditor->files[peditor->fileIdx]->data.bUpdateAll = true;
	fData_refreshEdit(peditor);
}

static inline void s_femto_inner_saveAs(fData_t * restrict peditor, wchar * restrict tempstr)
{
	fFile_t * restrict pfile = peditor->files[peditor->fileIdx];

	wcscpy_s(tempstr, MAX_STATUS, L"Save as... :");
	fData_statusMsg(peditor, tempstr, NULL);

	wchar inp[MAX_STATUS];
	if (femto_askInput(peditor, inp, MAX_STATUS))
	{
		wchar * restrict oldfilename = pfile->fileName;
		pfile->fileName = inp;
		const isize saved = fFile_write(pfile);
		switch (saved)
		{
		case ffwrNOTHING_NEW:
			wcscpy_s(tempstr, MAX_STATUS, L"Nothing new to save");
			break;
		case ffwrOPEN_ERROR:
			wcscpy_s(tempstr, MAX_STATUS, L"File open error!");
			break;
		case ffwrWRITE_ERROR:
			wcscpy_s(tempstr, MAX_STATUS, L"File is write-protected!");
			break;
		case ffwrMEM_ERROR:
			wcscpy_s(tempstr, MAX_STATUS, L"Memory allocation error!");
			break;
		default:
			swprintf_s(tempstr, MAX_STATUS, L"Wrote %zd bytes to %s", saved, inp);
			// Set console title
			femto_setConTitle(inp);
		}

		switch (saved)
		{
		case ffwrOPEN_ERROR:
		case ffwrWRITE_ERROR:
		case ffwrMEM_ERROR:
			// Revert back to old name
			pfile->fileName = oldfilename;
			break;
		default:
			// Switch to the new name
			pfile->fileName = wcsredup(oldfilename, inp);
		}
	}
	else
	{
		wcscpy_s(tempstr, MAX_STATUS, L"Saving canceled by user");
	}
}
static inline void s_femto_inner_searchTerm(fData_t * restrict peditor, wchar * restrict tempstr, bool first)
{
	if (peditor->psearchTerm == NULL)
	{
		wcscpy_s(tempstr, MAX_STATUS, L"No search term entered");
		return;
	}
	else
	{
		fFile_t * restrict pfile = peditor->files[peditor->fileIdx];
		assert(pfile != NULL);

		// Move cursor to the next result according to direction
		const isize delta = (peditor->bDirBack) ? -1 : 1;

		fLine_t * restrict node = pfile->data.currentNode;
		if (node == NULL)
		{
			wcscpy_s(tempstr, MAX_STATUS, L"No lines to be searched");
			return;
		}
		//
		node = first ? node : ((peditor->bDirBack) ? node->prevNode : node->nextNode);
		isize deltaLines = first ? 0 : delta;

		if (first)
		{
			pfile->data.bUpdateAll = true;
			fData_refreshEdit(peditor);
		}

		while (node != NULL)
		{
			if (node->userValue.bits.b8)
			{
				// Go to line
				if (deltaLines != 0)
				{
					fLine_moveCursorVert(&pfile->data.currentNode, deltaLines);
				}
				node->userValue.bits.b7 = true;
				pfile->data.bTyped = true;
				// Refresh search result highlighting, move cursor in place, calculate virtual cursor position
				fData_refreshEdit(peditor);
				// Update cursor horizontal position correctly
				fData_refreshEdit(peditor);
				swprintf_s(tempstr, MAX_STATUS, L"Found at line %u", node->lineNumber);

				return;
			}

			node = (peditor->bDirBack) ? node->prevNode : node->nextNode;
			deltaLines += delta;
		}
	}
	wcscpy_s(tempstr, MAX_STATUS, L"No more search results");
}
static inline void s_femto_inner_find(fData_t * restrict peditor, wchar * restrict tempstr, bool backward)
{
	wcscpy_s(tempstr, MAX_STATUS, backward ? L"Search backward: " : L"Search forward: ");
	fData_statusMsg(peditor, tempstr, NULL);

	if (femto_askInput(peditor, peditor->searchBuf, MAX_STATUS))
	{
		peditor->psearchTerm = (peditor->searchBuf[0] == L'\0') ? NULL : peditor->searchBuf;
		peditor->bDirBack    = backward;
		s_femto_inner_searchTerm(peditor, tempstr, true);
	}
	else
	{
		peditor->psearchTerm = NULL;
		wcscpy_s(tempstr, MAX_STATUS, L"Search cancelled by user");
		peditor->files[peditor->fileIdx]->data.bUpdateAll = true;
		fData_refreshEdit(peditor);
	}
}

static inline bool s_femto_inner_kbdHandle(
	fData_t * restrict peditor,
	const KEY_EVENT_RECORD * restrict ir
)
{
	assert(peditor != NULL);
	assert(ir != NULL);
	
	fFile_t * restrict pfile = peditor->files[peditor->fileIdx];
	assert(pfile != NULL);
	
	static wchar prevkey, prevwVirtKey;

	static u32 keyCount = 1;
	static bool waitingEnc = false;

	wchar key      = ir->uChar.UnicodeChar;
	wchar wVirtKey = ir->wVirtualKeyCode;
	const bool keydown = ir->bKeyDown != 0;

	if (keydown)
	{
		keyCount = ((key == prevkey) && (wVirtKey == prevwVirtKey)) ? (keyCount + 1) : 1;
		
		wchar tempstr[MAX_STATUS];
		bool draw = true;

		if (((wVirtKey == VK_ESCAPE) && (prevwVirtKey != VK_ESCAPE)) ||
			((key == sacCTRL_Q) && (key != sacCTRL_Q)) )	// Exit on Escape or Ctrl+Q
		{
			if (peditor->psearchTerm != NULL)
			{
				peditor->psearchTerm = NULL;
				wcscpy_s(tempstr, MAX_STATUS, L"Exited from search!");
				pfile->data.bUpdateAll = true;
				fData_refreshEdit(peditor);
			}
			else if (!s_femto_inner_quit(peditor, tempstr, key, L"Shift+ESC"))
			{
				return false;
			}
		}
		else if (waitingEnc && (key != sacCTRL_E))
		{
			bool done = true;
			switch (wVirtKey)
			{
			// CRLF
			case L'F':
				pfile->eolSeq = eolCRLF;
				break;
			// LF
			case L'L':
				pfile->eolSeq = eolLF;
				break;
			// CR
			case L'C':
				pfile->eolSeq = eolCR;
				break;
			default:
				wcscpy_s(tempstr, MAX_STATUS, L"Unknown EOL combination!");
				done = false;
			}
			if (done)
			{
				swprintf_s(
					tempstr,
					MAX_STATUS,
					L"Using %s%s EOL sequences",
					(pfile->eolSeq & eolCR) ? L"CR" : L"",
					(pfile->eolSeq & eolLF) ? L"LF" : L""
				);
			}

			waitingEnc = false;
		}
		else if ((key == sacCTRL_N) && (prevkey != sacCTRL_N))
		{
			s_femto_inner_openTab(peditor, tempstr, NULL);
		}
		else if (key == sacCTRL_O)
		{
			wcscpy_s(tempstr, MAX_STATUS, L"Open :");
			fData_statusMsg(peditor, tempstr, NULL);

			wchar inp[MAX_STATUS];
			if (femto_askInput(peditor, inp, MAX_STATUS))
			{
				// Try to create new tab and open file
				s_femto_inner_openTab(peditor, tempstr, inp);
			}
			else
			{
				wcscpy_s(tempstr, MAX_STATUS, L"Open canceled by user");
			}
		}
		else if ((key == sacCTRL_W) && (prevkey != sacCTRL_W))
		{
			if (peditor->filesSize == 1)
			{
				if (!s_femto_inner_quit(peditor, tempstr, key, L"Ctrl+Shift+W"))
				{
					return false;
				}
			}
			else
			{
				s_femto_inner_closeTab(peditor, tempstr, false);
			}
		}
		else if ((key == sacCTRL_R) && (prevkey != sacCTRL_R))	// Reload file
		{
			bool reload = true;
			if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000))
			{
				// Check for changes
				if (fFile_checkUnsaved(pfile, NULL, NULL) != ffcrNOTHING_NEW)
				{
					reload = false;
					wcscpy_s(tempstr, MAX_STATUS, L"Unsaved work detected. Press Ctrl+Shift+R to confirm reload");
				}
			}
			if (reload)
			{
				const wchar * restrict res = fFile_read(pfile);
				if (res != NULL)
				{
					wcscpy_s(tempstr, MAX_STATUS, res);
				}
				else
				{
					swprintf_s(
						tempstr,
						MAX_STATUS,
						L"File reloaded successfully! %s%s EOL sequences",
						(pfile->eolSeq & eolCR) ? L"CR" : L"",
						(pfile->eolSeq & eolLF) ? L"LF" : L""
					);
				}
				fData_refreshEdit(peditor);
			}
		}
		else if ((key == sacCTRL_S) && (prevkey != sacCTRL_S))	// Save file
		{
			if (pfile->fileName == NULL)
			{
				s_femto_inner_saveAs(peditor, tempstr);
			}
			else
			{
				const isize saved = fFile_write(pfile);
				switch (saved)
				{
				case ffwrNOTHING_NEW:
					wcscpy_s(tempstr, MAX_STATUS, L"Nothing new to save");
					break;
				case ffwrOPEN_ERROR:
					wcscpy_s(tempstr, MAX_STATUS, L"File open error!");
					break;
				case ffwrWRITE_ERROR:
					wcscpy_s(tempstr, MAX_STATUS, L"File is write-protected!");
					break;
				case ffwrMEM_ERROR:
					wcscpy_s(tempstr, MAX_STATUS, L"Memory allocation error!");
					break;
				default:
					swprintf_s(tempstr, MAX_STATUS, L"Wrote %zd bytes", saved);
				}
			}
		}
		else if ((key == sacCTRL_E) && (prevkey != sacCTRL_E))
		{
			waitingEnc = true;
			wcscpy_s(tempstr, MAX_STATUS, L"Waiting for EOL combination (F = CRLF, L = LF, C = CR)...");
		}
		else if (key == sacCTRL_F)
		{
			s_femto_inner_find(peditor, tempstr, false);
		}
		// Normal keys
		else if (key > sacLAST_CODE)
		{
			swprintf_s(tempstr, MAX_STATUS, L"'%c' #%u", key, keyCount);
			if (fFile_addNormalCh(pfile, key, peditor->settings.tabWidth))
			{
				fData_refreshEditAsync(peditor);
			}
		}
		// Special keys
		else
		{
			bool send = true;
			switch (wVirtKey)
			{
			// Save as...
			case L'S':
				if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) &&
					((GetAsyncKeyState(VK_SHIFT)) & 0x8000) && (prevwVirtKey != L'S') )
				{
					send = false;
					s_femto_inner_saveAs(peditor, tempstr);
				}
				break;
			case L'W':
				if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) &&
					(GetAsyncKeyState(VK_SHIFT) & 0x8000) && (prevwVirtKey != L'W') )
				{
					send = false;
					if (peditor->filesSize == 1)
					{
						if (!s_femto_inner_quit(peditor, tempstr, key, L"Ctrl+Shift+W"))
						{
							return false;
						}
					}
					else
					{
						s_femto_inner_closeTab(peditor, tempstr, true);
					}
				}
				break;
			case L'R':
				if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) &&
					(GetAsyncKeyState(VK_SHIFT) & 0x8000) && (prevwVirtKey != L'R'))
				{
					send = false;
					const wchar * restrict res = fFile_read(pfile);
					if (res != NULL)
					{
						wcscpy_s(tempstr, MAX_STATUS, res);
					}
					else
					{
						swprintf_s(
							tempstr,
							MAX_STATUS,
							L"File reloaded successfully! %s%s EOL sequences",
							(pfile->eolSeq & eolCR) ? L"CR" : L"",
							(pfile->eolSeq & eolLF) ? L"LF" : L""
						);
					}
					fData_refreshEdit(peditor);
				}
				break;
			case VK_TAB:
			{
				const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
				
				// Shuffle between tabs
				if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))
				{
					send = false;
					if (shift)
					{
						swprintf_s(tempstr, MAX_STATUS, L"Previous tab #%u", keyCount);
						--peditor->fileIdx;
						peditor->fileIdx = (peditor->fileIdx < 0) ? (isize)peditor->filesSize - 1 : peditor->fileIdx;
					}
					else
					{
						swprintf_s(tempstr, MAX_STATUS, L"Next tab #%u", keyCount);
						++peditor->fileIdx;
						peditor->fileIdx = (peditor->fileIdx >= (isize)peditor->filesSize) ? 0 : peditor->fileIdx;
					}
					pfile = peditor->files[peditor->fileIdx];
					
					if (peditor->filesSize > 1)
					{
						pfile->data.bUpdateAll = true;
						fData_refreshEdit(peditor);
						femto_setConTitle(pfile->fileName);
					}
				}
				else if (shift)
				{
					swprintf_s(tempstr, MAX_STATUS, L"\u2191 + 'TAB' #%u", keyCount);
					wVirtKey = VK_OEM_BACKTAB;
				}
				else
				{
					swprintf_s(tempstr, MAX_STATUS, L"'TAB' #%u", keyCount);
				}
				break;
			}
			case VK_F2:
			case VK_F3:
				send = false;
				peditor->bDirBack = (wVirtKey == VK_F2);
				s_femto_inner_searchTerm(peditor, tempstr, false);
				break;
			case VK_DELETE:
			{
				const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
				// Check for shift to alt key
				if (shift ^ ((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000)))
				{
					swprintf_s(tempstr, MAX_STATUS, L"%s + 'DEL' #%u", shift ? L"\u2191" : L"'ALT'", keyCount);
					wVirtKey = FEMTO_SHIFT_DEL;
				}
				else
				{
					swprintf_s(tempstr, MAX_STATUS, L"'DEL' #%u", keyCount);
				}
				break;
			}
			case VK_UP:		// Up arrow
				// Check for alt key
				if ((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))
				{
					swprintf_s(tempstr, MAX_STATUS, L"'ALT' + \u2191 #%u", keyCount);
					wVirtKey = FEMTO_MOVELINE_UP;
				}
				else
				{
					swprintf_s(tempstr, MAX_STATUS, L"\u2191 #%u", keyCount);
				}
				break;
			case VK_DOWN:	// Down arrow
				// Check for alt key
				if ((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(VK_RMENU) & 0x8000))
				{
					swprintf_s(tempstr, MAX_STATUS, L"'ALT' + \u2193 #%u", keyCount);
					wVirtKey = FEMTO_MOVELINE_DOWN;
				}
				else
				{
					swprintf_s(tempstr, MAX_STATUS, L"\u2193 #%u", keyCount);
				}
				break;
			case VK_RETURN:	// Enter key
			case VK_BACK:	// Backspace
			case VK_LEFT:	// Left arrow
			case VK_RIGHT:	// Right arrow
			case VK_PRIOR:	// Page up
			case VK_NEXT:	// Page down
			case VK_END:
			case VK_HOME:
			{
				static const wchar * buf[] = {
					[VK_RETURN] = L"'RET'",
					[VK_BACK]   = L"'BS'",
					[VK_LEFT]   = L"\u2190",
					[VK_RIGHT]  = L"\u2192",
					[VK_PRIOR]  = L"'PGUP'",
					[VK_NEXT]   = L"'PGDOWN'",
					[VK_END]	= L"'END'",
					[VK_HOME]   = L"'HOME'"
				};
				swprintf_s(tempstr, MAX_STATUS, L"%s #%u", buf[wVirtKey], keyCount);
				break;
			}
			case VK_CAPITAL:
				wcscpy_s(tempstr, MAX_STATUS, (GetKeyState(VK_CAPITAL) & 0x0001) ? L"'CAPS' On" : L"'CAPS' Off");
				break;
			case VK_NUMLOCK:
				wcscpy_s(
					tempstr, MAX_STATUS,
					(GetKeyState(VK_NUMLOCK) & 0x0001) ? L"'NUMLOCK' On" : L"'NUMLOCK' Off"
				);
				break;
			case VK_SCROLL:
				wcscpy_s(
					tempstr, MAX_STATUS,
					(GetKeyState(VK_SCROLL) & 0x0001) ? L"'SCRLOCK' On" : L"'SCRLOCK' Off"
				);
				break;
			default:
				//draw = false;
				wcscpy_s(tempstr, MAX_STATUS, L"Unkown key combination!");
			}

			if (send && fFile_addSpecialCh(
				pfile, peditor->scrbuf.h,
				wVirtKey, (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0,
				&peditor->settings
			))
			{
				fData_refreshEdit(peditor);
			}
		}
		if (draw)
		{
			fData_statusMsg(peditor, tempstr, NULL);
		}
	}
	else
	{
		key = wVirtKey = 0;
	}

	prevkey      = key;
	prevwVirtKey = wVirtKey;

	return true;
}

static inline bool s_femto_inner_calcMousePos(
	const fData_t * restrict peditor,
	const MOUSE_EVENT_RECORD * restrict ir,
	COORD * restrict ppos
)
{
	assert(peditor != NULL);
	assert(ir != NULL);
	assert(ppos != NULL);

	const fFile_t * restrict pfile = peditor->files[peditor->fileIdx];
	assert(pfile != NULL);

	COORD pos = ir->dwMousePosition;
	const SHORT nolen = (SHORT)(pfile->data.noLen + 1);

	if ((pos.Y == (SHORT)peditor->scrbuf.h) || (pos.X < nolen))
	{
		return false;
	}

	pos.X -= nolen;
	*ppos = pos;
	return true;
}
static inline i32 s_femto_inner_calcMouseScroll(
	i32 * restrict s_delta,
	const MOUSE_EVENT_RECORD * restrict ir,
	bool * restrict deltaPositive
)
{
	assert(s_delta != NULL);
	assert(ir != NULL);
	assert(deltaPositive != NULL);

	const i32 delta = (i32)(i16)HIWORD(ir->dwButtonState);
	fProf_write("Mouse wheel was used, delta: %d", delta);
	*deltaPositive = (delta > 0);

	*s_delta = (((*s_delta > 0) && (delta < 0)) || ((*s_delta < 0) && (delta > 0))) ? delta : (*s_delta + delta);

	const i32 lineDelta = *s_delta / WHEEL_DELTA;
	*s_delta -= lineDelta * WHEEL_DELTA;

	return -2 * lineDelta;
}
static inline bool s_femto_inner_mouseHandle(
	fData_t * restrict peditor,
	const MOUSE_EVENT_RECORD * restrict ir
)
{
	assert(peditor != NULL);
	assert(ir != NULL);

	fFile_t * pfile = peditor->files[peditor->fileIdx];
	assert(pfile != NULL);

	wchar tempstr[MAX_STATUS];
	bool draw = true;
	const bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000);

	if ((ir->dwEventFlags & MOUSE_WHEELED) && !shift)
	{
		static i32 s_delta = 0;
		bool deltaPositive;
		const isize lineDelta = (isize)s_femto_inner_calcMouseScroll(&s_delta, ir, &deltaPositive);
		draw = (lineDelta != 0);

		if (draw)
		{
			fFile_scrollVert(pfile, peditor->scrbuf.h, lineDelta);
			fData_refreshEditAsync(peditor);
			swprintf_s(
				tempstr, MAX_STATUS,
				L"'WHEEL-%s', %d lines",
				deltaPositive ? L"UP" : L"DOWN", lineDelta
			);
		}
	}
	else if ((ir->dwEventFlags & MOUSE_HWHEELED) || ((ir->dwEventFlags & MOUSE_WHEELED) && shift))
	{
		static i32 s_delta = 0;
		bool deltaPositive;
		const isize chDelta = (isize)s_femto_inner_calcMouseScroll(&s_delta, ir, &deltaPositive);
		draw = (chDelta != 0);
		
		if (draw)
		{
			fFile_scrollHor(pfile, peditor->scrbuf.w, chDelta);
			fData_refreshEditAsync(peditor);
			swprintf_s(
				tempstr, MAX_STATUS,
				L"'HWHEEL-%s', %d characters",
				deltaPositive ? L"RIGHT" : L"LEFT", chDelta
			);
		}
	}
	// Mouse click
	else if (ir->dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
	{
		COORD pos;
		// Check whether the mouse is in the right portion of the window or not
		// If not, don't draw anything on the statusbar
		draw = s_femto_inner_calcMousePos(peditor, ir, &pos);

		// Check if mouse is moving
		if (ir->dwEventFlags & MOUSE_MOVED)
		{
			if (draw)
			{
				swprintf_s(tempstr, MAX_STATUS, L"'LCLICK' + MOVE @%hd, %hd", pos.X, pos.Y);
			}
		}
		else
		{
			if (draw)
			{
				fProf_write("Mouse click @%hd, %hd", pos.X, pos.Y);

				if (pfile->data.pcury != NULL)
				{
					pfile->data.currentNode = pfile->data.pcury;
					const fLine_t * restrict lastcurnode = pfile->data.currentNode;
					fLine_moveCursorVert(&pfile->data.currentNode, (isize)pos.Y);
					pfile->data.bUpdateAll |= (pfile->data.currentNode != lastcurnode) & peditor->settings.bRelLineNums;
					// Now move the cursor to correct X position
					fLine_moveCursorAbs(pfile->data.currentNode, fLine_calcCursor(pfile->data.currentNode, (usize)pos.X + pfile->data.curx, peditor->settings.tabWidth));
					fLine_calcVirtCursor(pfile->data.currentNode, peditor->settings.tabWidth);
					pfile->data.lastx = pfile->data.currentNode->virtcurx;
					fData_refreshEditAsync(peditor);
				}
				swprintf_s(tempstr, MAX_STATUS, L"'LCLICK' @%hd, %hd", pos.X, pos.Y);
			}
		}
	}
	else
	{
		draw = false;
	}
	
	if (draw)
	{
		fData_statusMsg(peditor, tempstr, NULL);
	}

	return true;
}

bool femto_loop(fData_t * restrict peditor)
{
	assert(peditor != NULL);

	INPUT_RECORD ir;
	DWORD evRead;
	if (!ReadConsoleInputW(peditor->conIn, &ir, 1, &evRead) || !evRead)
	{
		return true;
	}

	if (ir.EventType == KEY_EVENT)
	{
		return s_femto_inner_kbdHandle(peditor, &ir.Event.KeyEvent);
	}
	// Mouse wheel was used
	else if (ir.EventType == MOUSE_EVENT)
	{
		return s_femto_inner_mouseHandle(peditor, &ir.Event.MouseEvent);
	}

	return true;
}
DWORD WINAPI femto_asyncDraw(LPVOID pdataV)
{
	fData_t * restrict pdata = pdataV;
	assert(pdata != NULL);
	fDrawThreadData_t * dt = &pdata->drawThread;
	assert(dt != NULL);

	while (!dt->bKillSwitch)
	{
		EnterCriticalSection(&dt->crit);

		while (!dt->bReady)
		{
			SleepConditionVariableCS(&dt->cv, &dt->crit, INFINITE);
		}
		dt->bReady = false;

		LeaveCriticalSection(&dt->crit);

		if (dt->bKillSwitch)
		{
			break;
		}

		// Do the drawing here
		fData_refreshEdit(pdata);
	}

	return 0;
}
bool femto_asyncDrawInit(fData_t * restrict pdata)
{
	assert(pdata != NULL);

	// Initialize thread resources

	fDrawThreadData_t * dt = &pdata->drawThread;

	// Create critical section (mutex), cv
	InitializeCriticalSection(&dt->crit);
	InitializeConditionVariable(&dt->cv);
	
	dt->bKillSwitch = false;
	dt->bReady = false;

	dt->hthread = CreateThread(
		NULL,
		20 * sizeof(usize),
		&femto_asyncDraw,
		pdata,
		0,
		NULL
	);

	// Check if CreateThread fails
	if (dt->hthread == NULL)
	{
		DeleteCriticalSection(&dt->crit);
		return false;
	}

	return true;
}
void femto_asyncDrawStop(fData_t * restrict pdata)
{
	assert(pdata != NULL);

	fDrawThreadData_t * dt = &pdata->drawThread;

	// Trigger thread killSwitch
	EnterCriticalSection(&dt->crit);

	dt->bKillSwitch = true;
	dt->bReady = true;
	WakeConditionVariable(&dt->cv);

	LeaveCriticalSection(&dt->crit);

	// Wait for thread to finish
	WaitForSingleObject(dt->hthread, INFINITE);

	// Free resources
	DeleteCriticalSection(&dt->crit);
}

bool femto_updateScrbuf(fData_t * restrict peditor, u32 * restrict curline)
{
	assert(peditor != NULL);
	assert(curline != NULL);
	assert(peditor->scrbuf.mem != NULL);
	
	fFile_t * restrict pfile = peditor->files[peditor->fileIdx];
	assert(pfile != NULL);

	// Count to current line
	u32 line = 0;
	fLine_t * restrict node = pfile->data.pcury;
	while ((node != NULL) && (node != pfile->data.currentNode))
	{
		node = node->nextNode;
		++line;
	}
	if (femto_updateScrbufLine(peditor, pfile->data.currentNode, line) == false)
	{
		// Whole screen buffer will be updated anyways
		node = pfile->data.pcury;
		bool drawCursor = false;
		for (u32 i = 0; i < peditor->scrbuf.h; ++i)
		{
			femto_updateScrbufLine(peditor, node, i);
			
			if (node == pfile->data.currentNode)
			{
				drawCursor = true;
			}
			if (node != NULL)
			{
				node = node->nextNode;
			}
		}
		// Update syntax for all lines
		node = pfile->data.firstNode;
		while (node != NULL)
		{
			fLine_updateSyntax(node, pfile->syntax, peditor->settings.syntaxColors, peditor->psearchTerm, peditor->settings.tabWidth);

			node = node->nextNode;
		}

		CONSOLE_CURSOR_INFO cci = { 0 };
		GetConsoleCursorInfo(peditor->scrbuf.handle, &cci);

		if ((drawCursor == false) && (cci.bVisible == TRUE))
		{
			cci.bVisible = FALSE;
			SetConsoleCursorInfo(peditor->scrbuf.handle, &cci);
		}

		return false;
	}

	*curline = line;
	return true;
}
bool femto_updateScrbufLine(fData_t * restrict peditor, fLine_t * restrict node, u32 line)
{
	assert(peditor != NULL);
	assert(peditor->scrbuf.mem != NULL);
	
	fFile_t * restrict pfile = peditor->files[peditor->fileIdx];
	assert(pfile != NULL);

	const fLine_t * restrict curnode = pfile->data.currentNode;
	
	if (pfile->data.bTyped || (pfile->data.pcury == NULL))
	{
		const usize prevcurx = pfile->data.curx;
		const fLine_t * restrict prevcury = pfile->data.pcury;

		pfile->data.bTyped = false;
		fFile_updateCury(pfile, peditor->scrbuf.h - 2);
		const isize delta = (isize)curnode->virtcurx - (isize)peditor->scrbuf.w - (isize)pfile->data.curx + (isize)pfile->data.noLen + 1;
		if (delta >= 0)
		{
			pfile->data.curx += (usize)(delta + 1);
		}
		else if (pfile->data.curx > curnode->virtcurx)
		{
			pfile->data.curx = max_usize(1, curnode->curx) - 1;
		}

		if ((prevcurx != pfile->data.curx) || (prevcury != pfile->data.pcury))
		{
			pfile->data.bUpdateAll = false;
			return false;
		}
	}
	if (pfile->data.bUpdateAll)
	{
		pfile->data.bUpdateAll = false;
		return false;
	}

	CHAR_INFO * restrict destination = &peditor->scrbuf.mem[(usize)line * (usize)peditor->scrbuf.w];
	for (u32 i = 0; i < peditor->scrbuf.w; ++i)
	{
		destination[i] = (CHAR_INFO){
			.Char       = { .UnicodeChar = L' ' },
			.Attributes = FEMTO_DEFAULT_COLOR
		};
	}
	if (node == NULL)
	{
		return true;
	}

	bool drawCursor = false;

	CONSOLE_CURSOR_INFO cci = { 0 };
	
	// if line is active line and cursor fits
	fLine_calcVirtCursor(node, peditor->settings.tabWidth);
	const usize curx = node->virtcurx - pfile->data.curx + pfile->data.noLen + 1;
	if ((node == pfile->data.currentNode) && (curx < peditor->scrbuf.w))
	{
		// Get cursor information
		GetConsoleCursorInfo(peditor->scrbuf.handle, &cci);

		// Update cursor position
		peditor->cursorpos[peditor->fileIdx] = (COORD){ .X = (SHORT)curx, .Y = (SHORT)line };
		// Make cursor visible, if necessary
		if (cci.bVisible == FALSE)
		{
			cci.bVisible = TRUE;
			SetConsoleCursorInfo(peditor->scrbuf.handle, &cci);
		}

		SetConsoleCursorPosition(peditor->scrbuf.handle, peditor->cursorpos[peditor->fileIdx]);
		drawCursor = true;
	}

	// Drawing

	// Advance idx by file.data.curx
	usize idx = 0;
	for (usize j = 0; (j < pfile->data.curx) && (idx < node->lineEndx);)
	{
		if ((idx == node->curx) && (node->freeSpaceLen > 0))
		{
			idx += node->freeSpaceLen;
			continue;
		}

		j += (node->line[idx] == L'\t') ? peditor->settings.tabWidth - (j % peditor->settings.tabWidth) : 1;
		++idx;
	}

	// Check to include tab character
	idx -= ((idx > 0) && (node->line[idx - 1] == L'\t') && ((pfile->data.curx % peditor->settings.tabWidth)));

	usize number = (!peditor->settings.bRelLineNums || (node == curnode)) ?
		(usize)node->lineNumber :
		(usize)labs((long)curnode->lineNumber - (long)node->lineNumber);
	const u8 noLen = (u8)log10((f64)number) + 1;
	destination[pfile->data.noLen].Attributes       = peditor->settings.lineNumCol;
	destination[pfile->data.noLen].Char.UnicodeChar = L'|';
	for (i8 j = (i8)pfile->data.noLen - 1; j >= 0; --j)
	{
		destination[j].Attributes = peditor->settings.lineNumCol;
		if (j >= (pfile->data.noLen - noLen))
		{
			destination[j].Char.UnicodeChar = (wchar)(number % 10) + L'0';
			number /= 10u;
		}
	}

	if (!fLine_updateSyntax(node, pfile->syntax, peditor->settings.syntaxColors, peditor->psearchTerm, peditor->settings.tabWidth))
	{
		fData_statusMsg(peditor, L"Error refreshing syntax highlighting!", NULL);
	}

	for (usize startj = (usize)pfile->data.noLen + 1, j = startj; (idx < node->lineEndx) && (j < peditor->scrbuf.w);)
	{
		if ((idx == node->curx) && (node->freeSpaceLen > 0))
		{
			idx += node->freeSpaceLen;
			continue;
		}

		destination[j].Attributes = FEMTO_DEFAULT_COLOR;
		if (node->line[idx] == L'\t')
		{
			const usize realIdx = j - startj + pfile->data.curx;
			destination[j].Char.UnicodeChar = peditor->settings.bWhiteSpaceVis ? peditor->settings.whitespaceCh  : L' ';
			destination[j].Attributes       = peditor->settings.bWhiteSpaceVis ? peditor->settings.whitespaceCol : destination[j].Attributes;
			++j;
			for (usize end = j + peditor->settings.tabWidth - ((realIdx) % peditor->settings.tabWidth) - 1; (j < end) && (j < peditor->scrbuf.w); ++j)
			{
				destination[j].Char.UnicodeChar = L' ';
			} 
		}
		else
		{
			destination[j].Char.UnicodeChar = node->line[idx];
			const usize stxIdx = (idx > node->curx) ? (idx - node->freeSpaceLen) : idx;
			destination[j].Attributes = (node->syntax != NULL) ? node->syntax[stxIdx] : destination[j].Attributes;
			++j;
		}
		
		++idx;
	}

	// Hide cursor, if necessary
	if ((drawCursor == false) && (cci.bVisible == TRUE))
	{
		cci.bVisible = FALSE;
		SetConsoleCursorInfo(peditor->scrbuf.handle, &cci);
	}

	return true;
}

u32 femto_toutf16(const char * restrict utf8, int numBytes, wchar ** restrict putf16, usize * restrict sz)
{
	assert(utf8 != NULL);
	assert(putf16 != NULL);
	// Query the needed size
	const u32 size = (numBytes == 0) ? 1 : (u32)MultiByteToWideChar(
		CP_UTF8,
		MB_PRECOMPOSED,
		utf8,
		numBytes,
		NULL,
		0
	);
	// Try to allocate memory
	if ((sz != NULL) && (*sz < (usize)size))
	{
		vptr mem = realloc(*putf16, (usize)size * sizeof(wchar));
		if (mem == NULL)
		{
			return 0;
		}
		*putf16 = mem;
		*sz     = (usize)size;
	}
	else if ((*putf16 == NULL) || (sz == NULL))
	{
		*putf16 = malloc((usize)size * sizeof(wchar));
		if (*putf16 == NULL)
		{
			return 0;
		}
	}

	if (numBytes != 0)
	{
		// Make conversion
		MultiByteToWideChar(
			CP_UTF8,
			MB_PRECOMPOSED,
			utf8,
			numBytes,
			*putf16,
			(int)size
		);
	}
	else
	{
		(*putf16)[0] = L'\0';
	}
	return size;
}
u32 femto_toutf8(const wchar * restrict utf16, int numChars, char ** restrict putf8, usize * restrict sz)
{
	assert(utf16 != NULL);
	assert(putf8 != NULL);
	const u32 size = (numChars == 0) ? 1 : (u32)WideCharToMultiByte(
		CP_UTF8,
		0,
		utf16,
		numChars,
		NULL,
		0,
		NULL,
		NULL
	);

	// Alloc mem
	if ((sz != NULL) && (*sz < (usize)size))
	{
		vptr mem = realloc(*putf8, (usize)size * sizeof(char));
		if (mem == NULL)
		{
			return 0;
		}
		*putf8 = mem;
		*sz    = (usize)size;
	}
	else if ((*putf8 == NULL) || (sz == NULL))
	{
		*putf8 = malloc((usize)size * sizeof(char));
		if (*putf8 == NULL)
		{
			return 0;
		}
	}

	// Convert
	if (numChars != 0)
	{
		WideCharToMultiByte(
			CP_UTF8,
			0,
			utf16,
			numChars,
			*putf8,
			(int)size,
			NULL,
			NULL
		);
	}
	else
	{
		(*putf8)[0] = '\0';
	}
	return size;
}
usize femto_strnToLines(wchar * restrict utf16, usize chars, wchar *** restrict lines, eolSeq_e * restrict eolSeq)
{
	assert(utf16  != NULL);
	assert(lines  != NULL);
	assert(eolSeq != NULL);

	// Count number of newline characters (to count number of lines - 1)
	usize newlines = 1;
	
	// Set default EOL sequence
	*eolSeq = eolDEF;
	for (usize i = 0; i < chars; ++i)
	{
		if (utf16[i] == L'\r')
		{
			*eolSeq = ((i + 1 < chars) && (utf16[i+1] == L'\n')) ? eolCRLF : eolCR;
			++newlines;
			i += (*eolSeq == eolCRLF);
		}
		else if (utf16[i] == L'\n')
		{
			*eolSeq = eolLF;
			++newlines;
		}
	}
	*lines = malloc(newlines * sizeof(wchar *));
	if (*lines == NULL)
	{
		return 0;
	}

	const bool isCRLF = (*eolSeq == eolCRLF);
	usize starti = 0, j = 0;
	for (usize i = 0; i < chars; ++i)
	{
		if ((utf16[i] == L'\n') || (utf16[i] == L'\r'))
		{
			utf16[i] = L'\0';
			(*lines)[j] = &utf16[starti];
			starti = i + 1 + isCRLF;
			i += isCRLF;
			++j;
		}
	}
	(*lines)[j] = &utf16[starti];

	return newlines;
}

bool femto_testFile(const wchar * restrict filename)
{
	assert(filename != NULL);

	HANDLE h = CreateFileW(
		filename,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (h == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	else
	{
		CloseHandle(h);
		return true;
	}
}
HANDLE femto_openFile(const wchar * restrict fileName, bool writemode)
{
	assert(fileName != NULL);

	HANDLE hfile = CreateFileW(
		fileName,
		writemode ? GENERIC_WRITE : GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		writemode ? CREATE_ALWAYS : OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);

	return hfile;
}
const wchar * femto_readBytes(HANDLE hfile, char ** restrict bytes, usize * restrict bytesLen)
{
	assert(bytes    != NULL);
	assert(bytesLen != NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		return L"File opening error!";
	}

	usize fileSize;
	{
		LARGE_INTEGER li;
		fileSize = GetFileSizeEx(hfile, &li) ? (usize)li.QuadPart : 0;
	}

	if ((fileSize >= *bytesLen) || (*bytes == NULL))
	{
		vptr mem = realloc(*bytes, fileSize + 1);
		if (mem == NULL)
		{
			return L"Memory error!";
		}
		*bytes    = mem;
		*bytesLen = fileSize + 1;
	}

	const BOOL readFileRes = ReadFile(
		hfile,
		*bytes,
		(DWORD)fileSize,
		NULL,
		NULL
	);
	if (!readFileRes)
	{
		return L"File read error!";
	}
	// Add null terminator
	(*bytes)[fileSize] = '\0';

	return NULL;
}

void femto_setConTitle(const wchar * restrict fileName)
{
	fileName = (fileName == NULL) ? FEMTO_UNTITLED_NAME : fileName;

	wchar wndName[MAX_PATH];
	const usize fnamelen = wcslen(fileName);
	memcpy(wndName, fileName, fnamelen * sizeof(wchar));
	wcscpy_s(wndName + fnamelen, MAX_PATH - fnamelen, L" - femto");
	SetConsoleTitleW(wndName);
}

#include "femto.h"
#include "femtoData.h"

bool boolGet(uint8_t * restrict arr, size_t index)
{
	assert(arr != NULL);
	return (arr[index / 8] & (0x01 << (index % 8))) != 0;
}
void boolPut(uint8_t * restrict arr, size_t index, bool value)
{
	assert(arr != NULL);
	const uint8_t pattern = 0x01 << (index % 8);
	value ? (arr[index / 8] |= pattern) : (arr[index / 8] &= (uint8_t)~pattern);
}

int32_t i32Min(int32_t a, int32_t b)
{
	return (a < b) ? a : b;
}
int32_t i32Max(int32_t a, int32_t b)
{
	return (a < b) ? b : a;
}
int32_t i32Clamp(int32_t value, int32_t min, int32_t max)
{
	return (value < min) ? min : (value > max) ? max : value;
}
uint32_t u32Min(uint32_t a, uint32_t b)
{
	return (a < b) ? a : b;
}
uint32_t u32Max(uint32_t a, uint32_t b)
{
	return (a < b) ? b : a;
}
uint32_t u32Clamp(uint32_t value, uint32_t min, uint32_t max)
{
	return (value < min) ? min : (value > max) ? max : value;
}

char * femto_cpcat_s(char ** restrict pstr, size_t * restrict psize, size_t * plen, wchar_t cp)
{
	unsigned char conv[3];
	size_t len = 0;
	if (cp <= 0x7F)
	{
		len = 1;
		conv[0] = (unsigned char)cp;
	}
	else if (cp <= 0x07FF)
	{
		len = 2;
		conv[0] = (uint8_t)(0xC0 | ((cp >> 6) & 0x1F));
		conv[1] = (uint8_t)(0x80 | ( cp       & 0x3F));
	}
	else
	{
		len = 3;
		conv[0] = (uint8_t)(0xE0 | ((cp >> 12) & 0x0F));
		conv[1] = (uint8_t)(0x80 | ((cp >>  6) & 0x3F));
		conv[2] = (uint8_t)(0x80 | ( cp        & 0x3F));
	}

	// Add converted string to original array
	char * ret = dynstrncat_s(pstr, psize, *plen, (char *)conv, len);
	if (ret != NULL)
	{
		*plen += len;
	}
	return ret;
}


char * femto_escapeStr(const char * inp)
{
	assert(inp != NULL);
	return femto_escapeStr_s(inp, strlen(inp));
}
char * femto_escapeStr_s(const char * inp, size_t len)
{
	assert(inp != NULL);
	assert(len > 0);

	char * mem = NULL;
	size_t mlen = 0, mcap = 0;

	for (size_t i = 0; i < len; ++i)
	{
		switch (inp[i])
		{
		case '\\':
			if (i < (len - 1))
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
					if (i < (len - 4))
					{
						++i;

						size_t end = i;
						uint16_t value = 0;
						for (; ((end - i) <= 4) && (end < len); ++end)
						{
							char t = (char)tolower(inp[end]);
							if (!(((t >= '0') && (t <= '9')) || ((t >= 'a') && (t <= 'f'))))
							{
								break;
							}

							uint8_t digit = (t >= 'A') ? (uint8_t)(t - 'A' + 10) : (uint8_t)(t - '0');
							value = (uint16_t)(value * 16 + digit);
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
						uint16_t value = 0;
						size_t end = i;
						for (; ((end - i) <= 3) && (end < len); ++end)
						{
							if (!((inp[end] >= '0') && (inp[end] <= '7')))
							{
								break;
							}
							uint16_t newvalue = (uint16_t)(value * 8 + (inp[end] - '0'));
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


static femtoData_t * s_atExitData = NULL;

void femto_exitHandlerSetVars(femtoData_t * pdata)
{
	assert(pdata != NULL);
	s_atExitData = pdata;
}
void femto_exitHandler(void)
{
	// Clear resources
	femtoData_destroy(s_atExitData);
}

void femto_printHelp(const wchar_t * restrict app)
{
	assert(app != NULL);
	fwprintf(stderr, L"Correct usage:\n%S [options] [file]\n", app);
}
void femto_printHelpClue(const wchar_t * restrict app)
{
	assert(app != NULL);
	fwprintf(stderr, L"To show all available commands type:\n%S --help\n", app);
}

bool femto_askInput(femtoData_t * restrict peditor, wchar_t * restrict line, uint32_t maxLen)
{
	assert(peditor != NULL);
	assert(line != NULL);
	assert(maxLen > 0);
	maxLen = u32Min(maxLen, peditor->scrbuf.w + 1);

	femtoSettings_t * restrict pset = &peditor->settings;

	femtoLineNode_t temp;
	femtoLine_init(&temp);

	// Find first non-space on the last line
	CHAR_INFO * lastline = &peditor->scrbuf.mem[peditor->scrbuf.w * (peditor->scrbuf.h - 1)];

	int16_t sidx = (int16_t)(peditor->scrbuf.w - 1);
	for (; sidx > 0; --sidx)
	{
		if (lastline[sidx].Char.UnicodeChar != L' ')
		{
			++sidx;
			break;
		}
	}

	// Start asking input
	COORD cur = { .X = sidx, .Y = (SHORT)(peditor->scrbuf.h - 1) };
	SetConsoleCursorPosition(peditor->scrbuf.handle, cur);
	
	bool read = true, update = false, updateCur = false;
	while (1)
	{
		INPUT_RECORD ir;
		DWORD evRead;
		if (!ReadConsoleInputW(peditor->conIn, &ir, 1, &evRead) || !evRead)
		{
			continue;
		}
		else if (evRead)
		{
			FlushConsoleInputBuffer(peditor->conIn);
		}

		if ((ir.EventType == KEY_EVENT) && ir.Event.KeyEvent.bKeyDown)
		{
			update    = false;
			updateCur = false;
			wchar_t key      = ir.Event.KeyEvent.uChar.UnicodeChar;
			wchar_t wVirtKey = ir.Event.KeyEvent.wVirtualKeyCode;

			if (wVirtKey == VK_ESCAPE)
			{
				read = false;
				break;
			}
			else if (wVirtKey == VK_RETURN)
			{
				break;
			}
			else if (key > sac_last_code)
			{
				// If there's room in the array
				if ((temp.lineEndx - temp.freeSpaceLen + 1) < maxLen)
				{
					// Normal keys

					// add key
					if (!femtoLine_addChar(&temp, key, pset->tabWidth))
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
						femtoLine_calcVirtCursor(&temp, pset->tabWidth);
						update = true;
					}
					break;
				case VK_LEFT:
					if (temp.curx > 0)
					{
						femtoLine_moveCursor(&temp, -1);
						femtoLine_calcVirtCursor(&temp, pset->tabWidth);
						updateCur = true;
					}
					break;
				case VK_RIGHT:
					if ((temp.curx + temp.freeSpaceLen) < temp.lineEndx)
					{
						femtoLine_moveCursor(&temp, 1);
						femtoLine_calcVirtCursor(&temp, pset->tabWidth);
						updateCur = true;
					}
					break;
				case VK_HOME:
					femtoLine_moveCursor(&temp, -(int32_t)temp.lineEndx);
					femtoLine_calcVirtCursor(&temp, pset->tabWidth);
					updateCur = true;
					break;
				case VK_END:
					femtoLine_moveCursor(&temp, (int32_t)temp.lineEndx);
					femtoLine_calcVirtCursor(&temp, pset->tabWidth);
					updateCur = true;
					break;

				}
			}

			// Update last line
			if (update)
			{
				femtoLine_getTextLim(&temp, line, maxLen);
				femtoData_statusDraw(peditor, line, NULL);
			}
			if (update | updateCur)
			{
				// Update cursor position
				cur.X = (int16_t)temp.virtcurx;
				SetConsoleCursorPosition(peditor->scrbuf.handle, cur);
			}
		}
	}

	femtoLine_destroy(&temp);

	SetConsoleCursorPosition(peditor->scrbuf.handle, peditor->cursorpos);
	return read;
}
bool femto_loop(femtoData_t * restrict peditor)
{
	assert(peditor != NULL);
	femtoFile_t * restrict pfile = peditor->file;
	assert(pfile != NULL);

	INPUT_RECORD ir;
	DWORD evRead;
	if (!ReadConsoleInputW(peditor->conIn, &ir, 1, &evRead) || !evRead)
	{
		return true;
	}
	else if (evRead)
	{
		FlushConsoleInputBuffer(peditor->conIn);
	}

	if (ir.EventType == KEY_EVENT)
	{
		static uint8_t keybuffer[32] = { 0 }, prevkeybuffer[32] = { 0 };
		static wchar_t prevkey, prevwVirtKey;

		static uint32_t keyCount = 1;
		static bool waitingEnc = false;

		wchar_t key      = ir.Event.KeyEvent.uChar.UnicodeChar;
		wchar_t wVirtKey = ir.Event.KeyEvent.wVirtualKeyCode;
		bool keydown     = ir.Event.KeyEvent.bKeyDown != 0;

		if (keydown)
		{
			if ((key == prevkey) && (wVirtKey == prevwVirtKey))
			{
				++keyCount;
			}
			else
			{
				keyCount = 1;
			}
		}

		if (keydown)
		{
			boolPut(keybuffer, key, true);
			wchar_t tempstr[MAX_STATUS];
			bool draw = true;

			if ((wVirtKey == VK_ESCAPE) || (key == sac_Ctrl_Q))	// Exit on Escape or Ctrl+Q
			{
				if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				{
					return false;
				}
				uint32_t realLen = 0;

				realLen += (uint32_t)swprintf_s(tempstr, MAX_STATUS, L"Unsaved file(s): ");

				// Scan for any unsaved work
				bool unsavedAny = false;
				for (size_t i = 0; i < peditor->filesSize; ++i)
				{
					femtoFile_t * f = peditor->files[i];
					femtoFile_checkUnsaved(f, NULL, NULL);
					if (f->unsaved && (realLen < MAX_STATUS))
					{
						unsavedAny = true;
						realLen += (uint32_t)swprintf_s(tempstr + realLen, MAX_STATUS - realLen, L"%s; ", f->fileName);
					}
				}
				if (!unsavedAny)
				{
					return false;
				}
				else if (realLen < MAX_STATUS)
				{
					swprintf_s(tempstr + realLen, MAX_STATUS - realLen, L"Press %s to confirm exit", (key == sac_Ctrl_Q) ? L"Ctrl+Shift+Q" : L"Shift+ESC");
				}
			}
			else if (waitingEnc && (key != sac_Ctrl_E))
			{
				bool done = true;
				switch (wVirtKey)
				{
				// CRLF
				case L'F':
					pfile->eolSeq = EOL_CRLF;
					break;
				// LF
				case L'L':
					pfile->eolSeq = EOL_LF;
					break;
				// CR
				case L'C':
					pfile->eolSeq = EOL_CR;
					break;
				default:
					swprintf_s(tempstr, MAX_STATUS, L"Unknown EOL combination!");
					done = false;
				}
				if (done)
				{
					swprintf_s(
						tempstr,
						MAX_STATUS,
						L"Using %s%s EOL sequences",
						(pfile->eolSeq & EOL_CR) ? L"CR" : L"",
						(pfile->eolSeq & EOL_LF) ? L"LF" : L""
					);
				}

				waitingEnc = false;
			}
			else if ((key == sac_Ctrl_N) && !boolGet(prevkeybuffer, sac_Ctrl_N))
			{
				swprintf_s(tempstr, MAX_STATUS, L"Ctrl+N");
			}
			else if ((key == sac_Ctrl_O) && !boolGet(prevkeybuffer, sac_Ctrl_O))
			{
				swprintf_s(tempstr, MAX_STATUS, L"Ctrl+O");
			}
			else if ((key == sac_Ctrl_W) && !boolGet(prevkeybuffer, sac_Ctrl_W))
			{
				swprintf_s(tempstr, MAX_STATUS, L"Closed tab #%u", keyCount);
			}
			else if ((key == sac_Ctrl_R) && !boolGet(prevkeybuffer, sac_Ctrl_R))	// Reload file
			{
				bool reload = true;
				if (!(GetAsyncKeyState(VK_SHIFT) & 0x8000))
				{
					// Check for changes
					if (femtoFile_checkUnsaved(pfile, NULL, NULL) != checkRes_nothingNew)
					{
						reload = false;
						swprintf_s(tempstr, MAX_STATUS, L"Unsaved work detected. Press Ctrl+Shift+R to confirm reload");
					}
				}
				if (reload)
				{
					const wchar_t * res;
					if ((res = femtoFile_read(pfile)) != NULL)
					{
						swprintf_s(tempstr, MAX_STATUS, res);
					}
					else
					{
						swprintf_s(
							tempstr,
							MAX_STATUS,
							L"File reloaded successfully! %s%s EOL sequences",
							(pfile->eolSeq & EOL_CR) ? L"CR" : L"",
							(pfile->eolSeq & EOL_LF) ? L"LF" : L""
						);
					}
					femtoData_refresh(peditor);
				}
			}
			else if ((key == sac_Ctrl_S) && !boolGet(prevkeybuffer, sac_Ctrl_S))	// Save file
			{
				int32_t saved = femtoFile_write(pfile);
				switch (saved)
				{
				case writeRes_nothingNew:
					swprintf_s(tempstr, MAX_STATUS, L"Nothing new to save");
					break;
				case writeRes_openError:
					swprintf_s(tempstr, MAX_STATUS, L"File open error!");
					break;
				case writeRes_writeError:
					swprintf_s(tempstr, MAX_STATUS, L"File is write-protected!");
					break;
				case writeRes_memError:
					swprintf_s(tempstr, MAX_STATUS, L"Memory allocation error!");
					break;
				default:
					swprintf_s(tempstr, MAX_STATUS, L"Wrote %d bytes", saved);
				}
			}
			else if ((key == sac_Ctrl_E) && !boolGet(prevkeybuffer, sac_Ctrl_E))
			{
				waitingEnc = true;
				swprintf_s(tempstr, MAX_STATUS, L"Waiting for EOL combination (F = CRLF, L = LF, C = CR)...");
			}
			// Normal keys
			else if (key > sac_last_code)
			{
				swprintf_s(tempstr, MAX_STATUS, L"'%c' #%u", key, keyCount);
				if (femtoFile_addNormalCh(pfile, key, peditor->settings.tabWidth))
				{
					femtoData_refresh(peditor);
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
					if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) && ((GetAsyncKeyState(VK_SHIFT)) & 0x8000))
					{
						send = false;
						swprintf_s(tempstr, MAX_STATUS, L"Save as... :");
						femtoData_statusDraw(peditor, tempstr, NULL);

						wchar_t inp[MAX_STATUS];
						if (femto_askInput(peditor, inp, MAX_STATUS))
						{
							swprintf_s(tempstr, MAX_STATUS, L"Saved %s successfully!", inp);
						}
						else
						{
							swprintf_s(tempstr, MAX_STATUS, L"Saving canceled by user");
						}
					}
					break;
				case L'R':
					if (((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000)) && (GetAsyncKeyState(VK_SHIFT) & 0x8000))
					{
						send = false;
						const wchar_t * res;
						if ((res = femtoFile_read(pfile)) != NULL)
						{
							swprintf_s(tempstr, MAX_STATUS, res);
						}
						else
						{
							swprintf_s(
								tempstr,
								MAX_STATUS,
								L"File reloaded successfully! %s%s EOL sequences",
								(pfile->eolSeq & EOL_CR) ? L"CR" : L"",
								(pfile->eolSeq & EOL_LF) ? L"LF" : L""
							);
						}
						femtoData_refresh(peditor);
					}
					break;
				case VK_TAB:
				{
					bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
					
					// Shuffle between tabs
					if ((GetAsyncKeyState(VK_LCONTROL) & 0x8000) || (GetAsyncKeyState(VK_RCONTROL) & 0x8000))
					{
						send = false;
						if (shift)
						{
							swprintf_s(tempstr, MAX_STATUS, L"Previous tab #%u", keyCount);
						}
						else
						{
							swprintf_s(tempstr, MAX_STATUS, L"Next tab #%u", keyCount);
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
				case VK_DELETE:
				{
					bool shift = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
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
					static const wchar_t * buf[] = {
						[VK_RETURN] = L"'RET'",
						[VK_BACK]   = L"'BS'",
						[VK_LEFT]   = L"\u2190",
						[VK_RIGHT]  = L"\u2192",
						[VK_PRIOR]  = L"'PGUP'",
						[VK_NEXT]   = L"'PGDOWN'",
						[VK_END]	= L"'END'",
						[VK_HOME]   = L"'HOME'",
					};
					swprintf_s(tempstr, MAX_STATUS, L"%s #%u", buf[wVirtKey], keyCount);
					break;
				}
				case VK_CAPITAL:
					swprintf_s(tempstr, MAX_STATUS, L"'CAPS' %s", (GetKeyState(VK_CAPITAL) & 0x0001) ? L"On" : L"Off");
					break;
				case VK_NUMLOCK:
					swprintf_s(tempstr, MAX_STATUS, L"'NUMLOCK' %s", (GetKeyState(VK_NUMLOCK) & 0x0001) ? L"On" : L"Off");
					break;
				case VK_SCROLL:
					swprintf_s(tempstr, MAX_STATUS, L"'SCRLOCK' %s", (GetKeyState(VK_SCROLL) & 0x0001) ? L"On" : L"Off");
					break;
				default:
					draw = false;
				}

				if (send && femtoFile_addSpecialCh(pfile, peditor->scrbuf.h, wVirtKey, &peditor->settings))
				{
					femtoData_refresh(peditor);
				}
			}
			if (draw)
			{
				femtoData_statusDraw(peditor, tempstr, NULL);
			}
		}
		else
		{
			boolPut(keybuffer, key, false);
		}
		prevkey = key;
		prevwVirtKey = wVirtKey;
		memcpy(prevkeybuffer, keybuffer, 32 * sizeof(uint8_t));
	}
	// Mouse wheel was used
	else if (ir.EventType == MOUSE_EVENT)
	{
		wchar_t tempstr[MAX_STATUS];
		bool draw = true;

		if (ir.Event.MouseEvent.dwEventFlags & MOUSE_WHEELED)
		{
			static int32_t s_delta = 0;
			int32_t delta = (int32_t)(int16_t)HIWORD(ir.Event.MouseEvent.dwButtonState);
			writeProfiler("femto_loop", "Mouse wheel was used, delta: %d", delta);

			if ((s_delta > 0 && delta < 0) || (s_delta < 0 && delta > 0))
			{
				s_delta = 0;
			}
			s_delta += delta;

			int32_t lineDelta = 2 * s_delta / WHEEL_DELTA;
			if (lineDelta != 0)
			{
				s_delta -= lineDelta * WHEEL_DELTA / 2;
				femtoFile_scroll(pfile, peditor->scrbuf.h, -lineDelta);
				femtoData_refresh(peditor);
			}
			swprintf_s(tempstr, MAX_STATUS, L"'WHEEL-%s' %d, %d lines", (delta > 0) ? L"UP" : L"DOWN", delta, lineDelta);
		}
		else if (ir.Event.MouseEvent.dwEventFlags & MOUSE_HWHEELED)
		{
			static int32_t s_delta = 0;
			int32_t delta = (int32_t)(int16_t)HIWORD(ir.Event.MouseEvent.dwButtonState);
			writeProfiler("femto_loop", "Mouse hwhell was used, delta %d", delta);

			if ((s_delta > 0 && delta < 0) || (s_delta < 0 && delta > 0))
			{
				s_delta = 0;
			}
			s_delta += delta;

			int32_t chDelta = s_delta / WHEEL_DELTA;
			if (chDelta != 0)
			{
				s_delta -= chDelta * WHEEL_DELTA;
				femtoFile_scrollHor(pfile, peditor->scrbuf.w, chDelta);
				femtoData_refresh(peditor);
			}
			swprintf_s(tempstr, MAX_STATUS, L"'WHEEL-%s' %d, %d character", (delta > 0) ? L"RIGHT" : L"LEFT", delta, chDelta);
		}
		// Mouse click
		else if (ir.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
		{
			// Check if mouse is moving
			if (ir.Event.MouseEvent.dwEventFlags & MOUSE_MOVED)
			{
				COORD pos = ir.Event.MouseEvent.dwMousePosition;
				if ((pos.Y == (SHORT)peditor->scrbuf.h) || pos.X < (pfile->data.noLen + 1))
				{
					draw = false;
				}
				else
				{
					pos.X -= (SHORT)(pfile->data.noLen + 1);
					swprintf_s(tempstr, MAX_STATUS, L"'LCLICK' + MOVE @%hd, %hd", pos.X, pos.Y);
				}
			}
			else
			{
				COORD pos = ir.Event.MouseEvent.dwMousePosition;
				if ((pos.Y == (SHORT)peditor->scrbuf.h) || pos.X < (pfile->data.noLen + 1))
				{
					draw = false;
				}
				else
				{
					pos.X -= (SHORT)(pfile->data.noLen + 1);
					writeProfiler("femto_loop", "Mouse click @%hd, %hd", pos.X, pos.Y);

					if (pfile->data.pcury != NULL)
					{
						pfile->data.currentNode = pfile->data.pcury;
						const femtoLineNode_t * lastcurnode = pfile->data.currentNode;
						femtoLine_moveCursorVert(&pfile->data.currentNode, (int32_t)pos.Y);
						pfile->data.updateAll |= (pfile->data.currentNode != lastcurnode) & peditor->settings.lineNumRelative;
						// Now move the cursor to correct X position
						femtoLine_moveCursorAbs(pfile->data.currentNode, femtoLine_calcCursor(pfile->data.currentNode, (uint32_t)pos.X + pfile->data.curx, peditor->settings.tabWidth));
						femtoLine_calcVirtCursor(pfile->data.currentNode, peditor->settings.tabWidth);
						pfile->data.lastx = pfile->data.currentNode->virtcurx;
						femtoData_refresh(peditor);
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
			femtoData_statusDraw(peditor, tempstr, NULL);
		}
	}

	return true;
}
bool femto_updateScrbuf(femtoData_t * restrict peditor, uint32_t * curline)
{
	assert(peditor != NULL);
	assert(curline != NULL);
	assert(peditor->scrbuf.mem != NULL);
	femtoFile_t * restrict pfile = peditor->file;
	assert(pfile != NULL);

	// Count to current line
	uint32_t line = 0;
	femtoLineNode_t * node = pfile->data.pcury;
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
		for (uint32_t i = 0; i < peditor->scrbuf.h; ++i)
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
bool femto_updateScrbufLine(femtoData_t * restrict peditor, femtoLineNode_t * restrict node, uint32_t line)
{
	assert(peditor != NULL);
	assert(peditor->scrbuf.mem != NULL);
	femtoFile_t * restrict pfile = peditor->file;
	assert(pfile != NULL);

	femtoLineNode_t * curnode = pfile->data.currentNode;
	
	if (pfile->data.typed || (pfile->data.pcury == NULL))
	{
		uint32_t prevcurx          = pfile->data.curx;
		femtoLineNode_t * prevcury = pfile->data.pcury;

		pfile->data.typed = false;
		femtoFile_updateCury(pfile, peditor->scrbuf.h - 2);
		int32_t delta = (int32_t)curnode->virtcurx - (int32_t)peditor->scrbuf.w - (int32_t)pfile->data.curx + (int32_t)pfile->data.noLen + 1;
		if (delta >= 0)
		{
			pfile->data.curx += (uint32_t)(delta + 1);
		}
		else if (pfile->data.curx > curnode->virtcurx)
		{
			pfile->data.curx = u32Max(1, curnode->curx) - 1;
		}

		if ((prevcurx != pfile->data.curx) || (prevcury != pfile->data.pcury))
		{
			pfile->data.updateAll = false;
			return false;
		}
	}
	if (pfile->data.updateAll)
	{
		pfile->data.updateAll = false;
		return false;
	}

	CHAR_INFO * destination = &peditor->scrbuf.mem[line * peditor->scrbuf.w];
	for (uint32_t i = 0; i < peditor->scrbuf.w; ++i)
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
	femtoLine_calcVirtCursor(node, peditor->settings.tabWidth);
	uint32_t curx = node->virtcurx - pfile->data.curx + pfile->data.noLen + 1;
	if ((node == pfile->data.currentNode) && (curx < peditor->scrbuf.w))
	{
		// Get cursor information
		GetConsoleCursorInfo(peditor->scrbuf.handle, &cci);

		// Update cursor position
		peditor->cursorpos = (COORD){ .X = (int16_t)curx, .Y = (int16_t)line };
		// Make cursor visible, if necessary
		if (cci.bVisible == FALSE)
		{
			cci.bVisible = TRUE;
			SetConsoleCursorInfo(peditor->scrbuf.handle, &cci);
		}

		SetConsoleCursorPosition(peditor->scrbuf.handle, peditor->cursorpos);
		drawCursor = true;
	}

	// Drawing

	// Advance idx by file.data.curx
	uint32_t idx = 0;
	for (uint32_t j = 0; (j < pfile->data.curx) && (idx < node->lineEndx);)
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
	if ((idx > 0) && (node->line[idx - 1] == L'\t') && ((pfile->data.curx % peditor->settings.tabWidth)))
	{
		--idx;
	}

	uint32_t number = (!peditor->settings.lineNumRelative || (node == curnode)) ? (uint32_t)node->lineNumber : (uint32_t)labs((long)curnode->lineNumber - (long)node->lineNumber);
	uint8_t noLen = (uint8_t)log10((double)number) + 1;
	destination[pfile->data.noLen].Attributes       = peditor->settings.lineNumCol;
	destination[pfile->data.noLen].Char.UnicodeChar = L'|';
	for (int8_t j = (int8_t)pfile->data.noLen - 1; j >= 0; --j)
	{
		destination[j].Attributes = peditor->settings.lineNumCol;
		if (j >= (pfile->data.noLen - noLen))
		{
			destination[j].Char.UnicodeChar = (wchar_t)(number % 10) + L'0';
			number /= 10u;
		}
	}
	for (uint32_t j = (uint8_t)pfile->data.noLen + 1; (idx < node->lineEndx) && (j < peditor->scrbuf.w);)
	{
		if ((idx == node->curx) && (node->freeSpaceLen > 0))
		{
			idx += node->freeSpaceLen;
			continue;
		}
		destination[j].Attributes = FEMTO_DEFAULT_COLOR;
		if (node->line[idx] == L'\t')
		{
			uint32_t realIdx = j + pfile->data.curx;
			destination[j].Char.UnicodeChar = peditor->settings.whitespaceVisible ? peditor->settings.whitespaceCh  : L' ';
			destination[j].Attributes       = peditor->settings.whitespaceVisible ? peditor->settings.whitespaceCol : destination[j].Attributes;
			++j;
			for (uint32_t end = j + peditor->settings.tabWidth - ((realIdx) % peditor->settings.tabWidth) - 1; (j < end) && (j < peditor->scrbuf.w); ++j)
			{
				destination[j].Char.UnicodeChar = L' ';
			} 
		}
		else
		{
			destination[j].Char.UnicodeChar = node->line[idx];
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

uint32_t femto_convToUnicode(const char * restrict utf8, int numBytes, wchar_t ** restrict putf16, uint32_t * restrict sz)
{
	assert(utf8 != NULL);
	assert(putf16 != NULL);
	// Query the needed size
	uint32_t size = (numBytes == 0) ? 1 : (uint32_t)MultiByteToWideChar(
		CP_UTF8,
		MB_PRECOMPOSED,
		utf8,
		numBytes,
		NULL,
		0
	);
	// Try to allocate memory
	if (sz != NULL && *sz < size)
	{
		void * mem = realloc(*putf16, size * sizeof(wchar_t));
		if (mem == NULL)
		{
			return 0;
		}
		*putf16 = mem;
		*sz     = size;
	}
	else if (*putf16 == NULL || sz == NULL)
	{
		*putf16 = malloc(size * sizeof(wchar_t));
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
uint32_t femto_convFromUnicode(const wchar_t * restrict utf16, int numChars, char ** restrict putf8, uint32_t * restrict sz)
{
	assert(utf16 != NULL);
	assert(putf8 != NULL);
	uint32_t size = (numChars == 0) ? 1 : (uint32_t)WideCharToMultiByte(
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
	if (sz != NULL && *sz < size)
	{
		void * mem = realloc(*putf8, size * sizeof(char));
		if (mem == NULL)
		{
			return 0;
		}
		*putf8 = mem;
		*sz    = size;
	}
	else if (*putf8 == NULL || sz == NULL)
	{
		*putf8 = malloc(size * sizeof(char));
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
uint32_t femto_strnToLines(wchar_t * restrict utf16, uint32_t chars, wchar_t *** restrict lines, enum femtoEOLsequence * restrict eolSeq)
{
	assert(utf16 != NULL);
	assert(lines != NULL);
	assert(eolSeq != NULL);

	// Count number of newline characters (to count number of lines - 1)
	uint32_t newlines = 1;
	
	// Set default EOL sequence
	*eolSeq = EOL_def;
	for (uint32_t i = 0; i < chars; ++i)
	{
		if (utf16[i] == L'\r')
		{
			*eolSeq = ((i + 1 < chars) && (utf16[i+1] == L'\n')) ? EOL_CRLF : EOL_CR;
			++newlines;
			i += (*eolSeq == EOL_CRLF);
		}
		else if (utf16[i] == L'\n')
		{
			*eolSeq = EOL_LF;
			++newlines;
		}
	}
	*lines = malloc(newlines * sizeof(wchar_t *));
	if (*lines == NULL)
	{
		return 0;
	}

	bool isCRLF = (*eolSeq == EOL_CRLF);
	uint32_t starti = 0, j = 0;
	for (uint32_t i = 0; i < chars; ++i)
	{
		if (utf16[i] == L'\n' || utf16[i] == L'\r')
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
uint32_t femto_tabsToSpaces(wchar_t ** restrict str, uint32_t * restrict len, uint8_t tabWidth)
{
	assert(str != NULL);
	assert(*str != NULL);
	assert((tabWidth > 1) && (tabWidth <= 32));
	uint32_t realLen = ((len == NULL || *len == 0) ? (uint32_t)wcslen(*str) + 1 : *len);
	uint32_t realCap = realLen;

	// Conversion happens here
	wchar_t * s = *str;

	for (uint32_t i = 0; i < realLen;)
	{
		if (s[i] == L'\t')
		{
			if ((realLen + tabWidth - 1) > realCap)
			{
				realCap = (realLen + tabWidth - 1) * 2;
				s = realloc(s, sizeof(wchar_t) * realCap);
				if (s == NULL)
				{
					return 0;
				}
			}
			memmove(&s[i + tabWidth - 1], &s[i], sizeof(wchar_t) * (realLen - i));
			for (uint32_t j = 0; j < tabWidth; ++i, ++j)
			{
				s[i] = L'\t';
			}
			realLen += tabWidth - 1;
		}
		else
		{
			++i;
		}
	}

	*str = s;
	// Shrink string
	s = realloc(s, sizeof(wchar_t) * realLen);
	if (s != NULL)
	{
		*str = s;
	}
	
	if (len != NULL)
	{
		*len = realLen;
	}
	return realLen;
}
uint32_t femto_spacesToTabs(wchar_t ** restrict str, uint32_t * restrict len, uint8_t tabWidth)
{
	assert(str != NULL);
	assert(*str != NULL);
	assert((tabWidth >= 1) && (tabWidth <= 32));
	
	uint32_t realLen = ((len == NULL || *len == 0) ? (uint32_t)wcslen(*str) + 1 : *len);

	wchar_t * s = *str;

	for (uint32_t i = 0; i < realLen; ++i)
	{
		if (s[i] == L'\t' && ((i + tabWidth - 1) < realLen))
		{
			bool convert = true;
			for (uint32_t j = 1; j < tabWidth; ++j)
			{
				if (s[i+j] != L'\t')
				{
					convert = false;
				}
			}
			if (!convert)
			{
				continue;
			}

			memmove(&s[i + 1], &s[i + tabWidth], sizeof(wchar_t) * (realLen - i - tabWidth));
			realLen -= tabWidth - 1;
		}
	}

	s = realloc(s, sizeof(wchar_t) * realLen);
	if (s != NULL)
	{
		*str = s;
	}

	if (len != NULL)
	{
		*len = realLen;
	}
	return realLen;
}

bool femto_testFile(const wchar_t * filename)
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

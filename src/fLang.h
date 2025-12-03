#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "fCommon.h"

typedef enum fLangId
{
	flangHELP_GENERAL,
	flangHELP_INDENDATION,
	flangHELP_AESTHETICS,
	flangHELP_MISCELLANEOUS,
	flangHELP_CLUE,
	flangHELP_KEYWORD,
	flangCATEGORIES,
	flangUSAGE,

	flangFILE,
	flangUNTITLED,
	flangUNSAVED_FILES,
	flangIS_UNSAVED,
	flangPRESS,
	flangCONFIRM_CLOSE,
	flangCONFIRM_EXIT,
	flangCONFIRM_RELOAD,
	flangCLOSED_TAB,
	flangOPT_FILE,
	flangNOTHING_NEW,
	flangOPEN_ERROR,
	flangWRITE_ERROR,
	flangMEM_ERROR,
	flangWROTE,
	flangBYTES,
	flangBYTES_TO,
	flangUNKNOWN_COMB,
	flangCAPS_ON,
	flangCAPS_OFF,
	flangLOAD,
	flangRELOAD,
	flangEOL,
	flangEOL_WAIT,
	flangCUT_ERROR,
	flangCUT,
	flangCUT_NOTHING,
	flangCOPY_ERROR,
	flangCOPY,
	flangCOPY_NOTHING,
	flangPASTE,
	flangPASTE_ERROR,
	flangPREV_TAB,
	flangNEXT_TAB,
	flangTAB,
	flangNEW_TAB,
	flangSYNTAX,
	flangSETTINGS,
	flangOPEN_FAIL,
	flangOPENED,
	flangSUCCESSFULLY,
	flangFOUND,
	flangSAVEAS,
	flangSAVE_CANCEL,
	flangSEARCH_NOTERM,
	flangSEARCH_NOLINES,
	flangSEARCH_END,
	flangSEARCH_FORWARD,
	flangSEARCH_BACKWARD,
	flangSEARCH_CANCEL,
	flangEOL_UNKNOWN,
	flangUSING,
	flangOPEN_CANCEL,
	flangMOUSE_LEFT,
	flangMOUSE_RIGHT,
	flangMOVE,
	flangOPEN,
	flangNUMLOCK_ON,
	flangNUMLOCK_OFF,
	flangSCRLOCK_ON,
	flangSCRLOCK_OFF,
	flangBTNRETURN,
	flangBTNBACKSPACE,
	flangBTNPGUP,
	flangBTNPGDN,
	flangBTNEND,
	flangBTNHOME,
	flangBTNINS,
	flangBTNDEL,
	flangBTNALT,
	flangBTNCTRL,

	flang_size

} fLangId_e;

typedef wchar *fLangArr_t[flang_size];
typedef const wchar *fCLangArr_t[flang_size];
typedef const char *fCLangArr8_t[flang_size];

extern const fCLangArr8_t fLang_tokens;
extern fCLangArr_t fLang_strings;

/**
 * @brief Returns pointer to language string
 *
 */
#define fLang_get(id) (fLang_strings[id])

bool fLang_init(void);
isize fLang_setLang8(const char *restrict language);
isize fLang_setLang(const wchar *restrict language);
bool fLang_addKeyword8(isize langId, fLangId_e kwId, const char *restrict keyword);
bool fLang_addKeyword(isize langId, fLangId_e kwId, const wchar *restrict keyword);
void fLang_destroy(void);

// Defined defaults

#define FLANG_HELP_GENERAL_DEF                            \
	L"\nGeneral:\n\n"                                     \
	"  --h                 Shows this help\n"             \
	"  --h=[value]         Shows help for a keyword\n"    \
	"  --file=[file]       Specifies file name to open\n" \
	"  --setting=[file]    Specifies custom settings file explicitly\n"

#define FLANG_HELP_INDENDATION_DEF                               \
	L"\nIndendation:\n\n"                                        \
	"  --tabsToSpaces=[value]    true/false, 1/0\n"              \
	"  --tabWidth=[value]        Number from 1-32 (inclusive)\n" \
	"  --autoIndent=[value]      true/false, 1/0\n"

#define FLANG_HELP_AESTHETICS_DEF                                                 \
	L"\nAesthetics:\n\n"                                                          \
	"  --whitespaceVisible=[value]    true/false, 1/0\n"                          \
	"  --whitespaceCh=[value]         Specifies 'visible' whitespace character\n" \
	"  --whitespaceCol=[value]        Specifies whitespace color, 0-255\n"        \
	"  --lineNumRelative=[value]      true/false, 1/0\n"                          \
	"  --lineNumCol=[value]           Specifies line number color, 0-255\n"

#define FLANG_HELP_MISCELLANEOUS_DEF \
	L"\nMiscellaneous:\n\n"          \
	"  --log=[value] Specifies the use of logging mode\n"

#define FLANG_HELP_KEYWORD L"Help keyword"
#define FLANG_HELP_CLUE L"To show all available commands type"
#define FLANG_CATEGORIES L"By categories"
#define FLANG_USAGE L"Usage:"

#define FLANG_FILE L"File"
#define FLANG_UNTITLED_NAME L"untitled"
#define FLANG_UNSAVED_FILES L"Unsaved file(s)"
#define FLANG_IS_UNSAVED L"is unsaved"
#define FLANG_PRESS L"Press"
#define FLANG_CONFIRM_CLOSE L"to confirm closing"
#define FLANG_CONFIRM_EXIT L"to confirm exit"
#define FLANG_CONFIRM_RELOAD L"Unsaved work detected. Press Ctrl+Shift+R to confirm reload"
#define FLANG_CLOSED_TAB L"Closed tab"
#define FLANG_OPT_FILE L"[options] [file]"
#define FLANG_NOTHING_NEW L"Nothing new to save"
#define FLANG_OPEN_ERROR L"File open error!"
#define FLANG_WRITE_ERROR L"The file is write-protected!"
#define FLANG_MEM_ERROR L"Memory allocation error!"
#define FLANG_WROTE L"Wrote"
#define FLANG_BYTES L"bytes"
#define FLANG_BYTES_TO L"bytes to"
#define FLANG_UNKNOWN_COMB L"Unknown key combination!"
#define FLANG_CAPS_ON L"'CAPS' On"
#define FLANG_CAPS_OFF L"'CAPS' Off"
#define FLANG_LOAD L"File loaded successfully!"
#define FLANG_RELOAD L"File reloaded successfully!"
#define FLANG_EOL L"EOL sequences"
#define FLANG_EOL_WAIT L"Waiting for EOL combination (F = CRLF, L = LF, C = CR)..."
#define FLANG_CUT_ERROR L"Cut error!"
#define FLANG_CUT L"Cut"
#define FLANG_CUT_NOTHING L"Nothing to cut"
#define FLANG_COPY L"Copy"
#define FLANG_COPY_ERROR L"Copy error!"
#define FLANG_COPY_NOTHING L"Nothing to copy"
#define FLANG_PASTE L"Paste"
#define FLANG_PASTE_ERROR L"Paste error!"
#define FLANG_PREV_TAB L"Previous tab"
#define FLANG_NEXT_TAB L"Next tab"
#define FLANG_TAB L"TAB"
#define FLANG_NEW_TAB L"new tab"
#define FLANG_SYNTAX L"Syntax"
#define FLANG_SETTINGS L"Settings file"
#define FLANG_OPEN_FAIL L"Failure while opening"
#define FLANG_OPENED L"Opened"
#define FLANG_SUCCESSFULLY L"successfully"
#define FLANG_FOUND L"Found"
#define FLANG_SAVEAS L"Save as"
#define FLANG_SAVE_CANCEL L"Saving canceled by user"
#define FLANG_SEARCH_NOTERM L"No search term entered"
#define FLANG_SEARCH_NOLINES L"No lines to be searched"
#define FLANG_SEARCH_END L"No more search results"
#define FLANG_SEARCH_FORWARD L"Search forward"
#define FLANG_SEARCH_BACKWARD L"Search backward"
#define FLANG_SEARCH_CANCEL L"Search cancelled by user"
#define FLANG_EOL_UNKNOWN L"Unknown EOL combination!"
#define FLANG_USING L"Using"
#define FLANG_OPEN_CANCEL L"Open canceled by user"
#define FLANG_MOUSE_LEFT L"LCLICK"
#define FLANG_MOUSE_RIGHT L"RCLICK"
#define FLANG_MOVE L"MOVE"
#define FLANG_OPEN L"Open"
#define FLANG_NUMLOCK_ON L"'NUMLOCK' On"
#define FLANG_NUMLOCK_OFF L"'NUMLOCK' Off"
#define FLANG_SCRLOCK_ON L"'SCRLOCK' On"
#define FLANG_SCRLOCK_OFF L"'SCRLOCK' Off"
#define FLANG_BTNRETURN L"'RET'"
#define FLANG_BTNBACKSPACE L"'BS'"
#define FLANG_BTNPGUP L"'PGUP'"
#define FLANG_BTNPGDN L"'PGDN'"
#define FLANG_BTNEND L"'END'"
#define FLANG_BTNHOME L"'HOME'"
#define FLANG_BTNINS L"'INS'"
#define FLANG_BTNDEL L"'DEL'"
#define FLANG_BTNALT L"'ALT'"
#define FLANG_BTNCTRL L"'CTRL'"

#endif

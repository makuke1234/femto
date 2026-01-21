#include "fLang.h"
#include "femto.h"

const fCLangArr8_t fLang_tokens = {
	[flangHELP_GENERAL] = "general",
	[flangHELP_INDENDATION] = "indent",
	[flangHELP_AESTHETICS] = "aesthetics",
	[flangHELP_MISCELLANEOUS] = "misc",
	[flangHELP_CLUE] = "clue",
	[flangHELP_KEYWORD] = "keyword",
	[flangCATEGORIES] = "categories",
	[flangUSAGE] = "usage",

	[flangFILE] = "file",
	[flangUNTITLED] = "untitled",
	[flangUNSAVED_FILES] = "unsaved-files",
	[flangIS_UNSAVED] = "is-unsaved",
	[flangPRESS] = "press",
	[flangCONFIRM_CLOSE] = "confirm-close",
	[flangCONFIRM_EXIT] = "confirm-exit",
	[flangCONFIRM_RELOAD] = "confirm-reload",
	[flangCLOSED_TAB] = "closed-tab",
	[flangOPT_FILE] = "opt-file",
	[flangNOTHING_NEW] = "nothing-new",
	[flangOPEN_ERROR] = "open-error",
	[flangWRITE_ERROR] = "write-error",
	[flangMEM_ERROR] = "mem-error",
	[flangWROTE] = "wrote",
	[flangBYTES] = "bytes",
	[flangBYTES_TO] = "bytes-to",
	[flangUNKNOWN_COMB] = "unknown-comb",
	[flangCAPS_ON] = "caps-on",
	[flangCAPS_OFF] = "caps-off",
	[flangLOAD] = "load",
	[flangRELOAD] = "reload",
	[flangEOL] = "eol",
	[flangEOL_WAIT] = "eol-wait",
	[flangCUT_ERROR] = "cut-error",
	[flangCUT] = "cut",
	[flangCUT_NOTHING] = "cut-nothing",
	[flangCOPY_ERROR] = "copy-error",
	[flangCOPY] = "copy",
	[flangCOPY_NOTHING] = "copy-nothing",
	[flangPASTE] = "paste",
	[flangPASTE_ERROR] = "paste-error",
	[flangPREV_TAB] = "prev-tab",
	[flangNEXT_TAB] = "next-tab",
	[flangTAB] = "tab",
	[flangNEW_TAB] = "new-tab",
	[flangSYNTAX] = "syntax",
	[flangSETTINGS] = "settings",
	[flangOPEN_FAIL] = "open-fail",
	[flangOPENED] = "opened",
	[flangSUCCESSFULLY] = "successfully",
	[flangFOUND] = "found",
	[flangSAVEAS] = "saveas",
	[flangSAVE_CANCEL] = "save-cancel",
	[flangSEARCH_NOTERM] = "search-noterm",
	[flangSEARCH_NOLINES] = "search-nolines",
	[flangSEARCH_END] = "search-end",
	[flangSEARCH_FORWARD] = "search-forward",
	[flangSEARCH_BACKWARD] = "search-backward",
	[flangSEARCH_CANCEL] = "search-cancel",
	[flangEOL_UNKNOWN] = "eol-unknown",
	[flangUSING] = "using",
	[flangOPEN_CANCEL] = "open-cancel",
	[flangMOUSE_LEFT] = "mouse-left",
	[flangMOUSE_RIGHT] = "mouse-right",
	[flangMOVE] = "move",
	[flangOPEN] = "open",
	[flangNUMLOCK_ON] = "numlock-on",
	[flangNUMLOCK_OFF] = "numlock-off",
	[flangSCRLOCK_ON] = "scrlock-on",
	[flangSCRLOCK_OFF] = "scrlock-off",
	[flangBTNRETURN] = "btn-ret",
	[flangBTNBACKSPACE] = "btn-bs",
	[flangBTNPGUP] = "btn-pgup",
	[flangBTNPGDN] = "btn-pgdn",
	[flangBTNEND] = "btn-end",
	[flangBTNHOME] = "btn-home",
	[flangBTNINS] = "btn-ins",
	[flangBTNDEL] = "btn-del",
	[flangBTNALT] = "btn-alt",
	[flangBTNCTRL] = "btn-ctrl",
};

#define fLang_string_contents                                 \
	[flangHELP_GENERAL] = FLANG_HELP_GENERAL_DEF,             \
	[flangHELP_INDENDATION] = FLANG_HELP_INDENDATION_DEF,     \
	[flangHELP_AESTHETICS] = FLANG_HELP_AESTHETICS_DEF,       \
	[flangHELP_MISCELLANEOUS] = FLANG_HELP_MISCELLANEOUS_DEF, \
	[flangHELP_CLUE] = FLANG_HELP_CLUE,                       \
	[flangHELP_KEYWORD] = FLANG_HELP_KEYWORD,                 \
	[flangCATEGORIES] = FLANG_CATEGORIES,                     \
	[flangUSAGE] = FLANG_USAGE,                               \
                                                              \
	[flangFILE] = FLANG_FILE,                                 \
	[flangUNTITLED] = FLANG_UNTITLED_NAME,                    \
	[flangUNSAVED_FILES] = FLANG_UNSAVED_FILES,               \
	[flangPRESS] = FLANG_PRESS,                               \
	[flangCONFIRM_CLOSE] = FLANG_CONFIRM_CLOSE,               \
	[flangCONFIRM_EXIT] = FLANG_CONFIRM_EXIT,                 \
	[flangCONFIRM_RELOAD] = FLANG_CONFIRM_RELOAD,             \
	[flangCLOSED_TAB] = FLANG_CLOSED_TAB,                     \
	[flangOPT_FILE] = FLANG_OPT_FILE,                         \
	[flangNOTHING_NEW] = FLANG_NOTHING_NEW,                   \
	[flangOPEN_ERROR] = FLANG_OPEN_ERROR,                     \
	[flangWRITE_ERROR] = FLANG_WRITE_ERROR,                   \
	[flangMEM_ERROR] = FLANG_MEM_ERROR,                       \
	[flangWROTE] = FLANG_WROTE,                               \
	[flangBYTES_TO] = FLANG_BYTES_TO,                         \
	[flangUNKNOWN_COMB] = FLANG_UNKNOWN_COMB,                 \
	[flangCAPS_ON] = FLANG_CAPS_ON,                           \
	[flangCAPS_OFF] = FLANG_CAPS_OFF,                         \
	[flangLOAD] = FLANG_LOAD,                                 \
	[flangRELOAD] = FLANG_RELOAD,                             \
	[flangEOL] = FLANG_EOL,                                   \
	[flangEOL_WAIT] = FLANG_EOL_WAIT,                         \
	[flangCUT_ERROR] = FLANG_CUT_ERROR,                       \
	[flangCUT] = FLANG_CUT,                                   \
	[flangCUT_NOTHING] = FLANG_CUT_NOTHING,                   \
	[flangCOPY_ERROR] = FLANG_COPY_ERROR,                     \
	[flangCOPY] = FLANG_COPY,                                 \
	[flangCOPY_NOTHING] = FLANG_COPY_NOTHING,                 \
	[flangPASTE] = FLANG_PASTE,                               \
	[flangPASTE_ERROR] = FLANG_PASTE_ERROR,                   \
	[flangPREV_TAB] = FLANG_PREV_TAB,                         \
	[flangNEXT_TAB] = FLANG_NEXT_TAB,                         \
	[flangTAB] = FLANG_TAB,                                   \
	[flangNEW_TAB] = FLANG_NEW_TAB,                           \
	[flangSYNTAX] = FLANG_SYNTAX,                             \
	[flangSETTINGS] = FLANG_SETTINGS,                         \
	[flangOPEN_FAIL] = FLANG_OPEN_FAIL,                       \
	[flangOPENED] = FLANG_OPENED,                             \
	[flangSUCCESSFULLY] = FLANG_SUCCESSFULLY,                 \
	[flangFOUND] = FLANG_FOUND,                               \
	[flangSAVEAS] = FLANG_SAVEAS,                             \
	[flangSAVE_CANCEL] = FLANG_SAVE_CANCEL,                   \
	[flangSEARCH_NOTERM] = FLANG_SEARCH_NOTERM,               \
	[flangSEARCH_NOLINES] = FLANG_SEARCH_NOLINES,             \
	[flangSEARCH_END] = FLANG_SEARCH_END,                     \
	[flangSEARCH_FORWARD] = FLANG_SEARCH_FORWARD,             \
	[flangSEARCH_BACKWARD] = FLANG_SEARCH_BACKWARD,           \
	[flangSEARCH_CANCEL] = FLANG_SEARCH_CANCEL,               \
	[flangEOL_UNKNOWN] = FLANG_EOL_UNKNOWN,                   \
	[flangUSING] = FLANG_USING,                               \
	[flangOPEN_CANCEL] = FLANG_OPEN_CANCEL,                   \
	[flangMOUSE_LEFT] = FLANG_MOUSE_LEFT,                     \
	[flangMOUSE_RIGHT] = FLANG_MOUSE_RIGHT,                   \
	[flangMOVE] = FLANG_MOVE,                                 \
	[flangOPEN] = FLANG_OPEN,                                 \
	[flangNUMLOCK_ON] = FLANG_NUMLOCK_ON,                     \
	[flangNUMLOCK_OFF] = FLANG_NUMLOCK_OFF,                   \
	[flangSCRLOCK_ON] = FLANG_SCRLOCK_ON,                     \
	[flangSCRLOCK_OFF] = FLANG_SCRLOCK_OFF,                   \
	[flangBTNRETURN] = FLANG_BTNRETURN,                       \
	[flangBTNBACKSPACE] = FLANG_BTNBACKSPACE,                 \
	[flangBTNPGUP] = FLANG_BTNPGUP,                           \
	[flangBTNPGDN] = FLANG_BTNPGDN,                           \
	[flangBTNEND] = FLANG_BTNEND,                             \
	[flangBTNHOME] = FLANG_BTNHOME,                           \
	[flangBTNINS] = FLANG_BTNINS,                             \
	[flangBTNDEL] = FLANG_BTNDEL,                             \
	[flangBTNALT] = FLANG_BTNALT,                             \
	[flangBTNCTRL] = FLANG_BTNCTRL,

static const fCLangArr_t s_fLang_defStrings = {fLang_string_contents};
fCLangArr_t fLang_strings = {fLang_string_contents};

static fLangArr_t *s_fLang_stringsArr = NULL;

static wchar **s_langNames = NULL;
static usize s_numLanguages = 0;
static isize s_langIdx = -1;

bool fLang_init(void)
{
	if ((s_numLanguages == 0) || (s_langNames == NULL) ||
		(s_fLang_stringsArr == NULL) || (s_langIdx >= (isize)s_numLanguages))
	{
		return false;
	}

	fLangArr_t *const strings = (s_langIdx == ((isize)-1)) ? NULL : &s_fLang_stringsArr[s_langIdx];

	for (usize i = 0; i < flang_size; ++i)
	{
		fLang_strings[i] = ((strings == NULL) || ((*strings)[i] == NULL)) ? s_fLang_defStrings[i] : (*strings)[i];
	}

	return true;
}
isize fLang_setLang8(const char *restrict language)
{
	wchar *lang16 = NULL;
	if (language != NULL)
	{
		femto_toutf16(language, (int)strlen(language) + 1, &lang16, NULL);
		if (lang16 == NULL)
		{
			return -1;
		}
	}
	isize ret = fLang_setLang(lang16);
	free(lang16);
	return ret;
}
isize fLang_setLang(const wchar *restrict language)
{
	if (language == NULL)
	{
		s_langIdx = -1;
		return s_langIdx;
	}

	// search for language in list
	const usize langLen = wcslen(language);
	if ((s_numLanguages != 0) && (s_langNames != NULL))
	{
		for (usize i = 0; i < s_numLanguages; ++i)
		{
			assert(s_langNames[i] != NULL);
			if (wcsncmp(s_langNames[i], language, langLen) == 0)
			{
				s_langIdx = (isize)i;
				return s_langIdx;
			}
		}
	}

	// add langauge to list, it wasn't found
	usize tempsz = s_numLanguages + 1;
	wchar **templan = realloc(s_langNames, sizeof(wchar *) * tempsz);
	if (templan == NULL)
	{
		return -1;
	}
	s_langNames = templan;
	s_numLanguages = tempsz;

	// resize s_fLang_string
	fLangArr_t *tempStr = realloc(s_fLang_stringsArr, sizeof(fLangArr_t) * s_numLanguages);
	if (tempStr == NULL)
	{
		--s_numLanguages;
		if (!s_numLanguages)
		{
			free(s_langNames);
			s_langNames = NULL;
		}
		else
		{
			s_langNames = realloc(s_langNames, sizeof(wchar *) * s_numLanguages);
		}
		return -1;
	}
	s_fLang_stringsArr = tempStr;

	for (usize i = 0; i < flang_size; ++i)
	{
		s_fLang_stringsArr[s_numLanguages - 1][i] = NULL;
	}

	s_langNames[s_numLanguages - 1] = wcsdup(language);
	s_langIdx = (isize)s_numLanguages - 1;

	return s_langIdx;
}
bool fLang_addKeyword8(isize langId, fLangId_e kwId, const char *restrict keyword)
{
	wchar *kw16 = NULL;
	if (keyword != NULL)
	{
		femto_toutf16(keyword, (int)strlen(keyword) + 1, &kw16, NULL);
		if (kw16 == NULL)
		{
			return -1;
		}
	}
	isize ret = fLang_addKeyword(langId, kwId, kw16);
	free(kw16);
	return ret;
}
bool fLang_addKeyword(isize langId, fLangId_e kwId, const wchar *restrict keyword)
{
	if ((langId >= (isize)s_numLanguages) || (kwId >= flang_size) || (keyword == NULL))
	{
		return false;
	}

	wchar *kwcopy = wcsdup(keyword);
	if (kwcopy == NULL)
	{
		return false;
	}

	s_fLang_stringsArr[langId][kwId] = kwcopy;

	return true;
}

void fLang_destroy(void)
{
	for (usize j = 0; j < s_numLanguages; ++j)
	{
		for (usize i = 0; i < flang_size; ++i)
		{
			if (s_fLang_stringsArr[j][i] != NULL)
			{
				free(s_fLang_stringsArr[j]);
			}
		}
		free(s_fLang_stringsArr[j]);
		free(s_langNames[j]);
	}

	free(s_fLang_stringsArr);
	free(s_langNames);
}

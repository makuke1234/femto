#ifndef FEMTOSETTINGS_H
#define FEMTOSETTINGS_H

#include "common.h"

#define FEMTO_SETTINGS_ERR_MAX 256

#define FEMTO_SETTINGS_FILE1 L"femto.json"
#define FEMTO_SETTINGS_FILE2 L"femto-settings.json"
#define FEMTO_SETTINGS_FILE3 L"femto_settings.json"
#define FEMTO_SETTINGS_FILE4 L"settings.json"

#define FEMTO_SETTINGS_MINTAB 1
#define FEMTO_SETTINGS_MAXTAB 32

#define FEMTO_SETTINGS_MINCOLOR 0
#define FEMTO_SETTINGS_MAXCOLOR UINT8_MAX


enum tokenColor
{
	tcTEXT,
	tcCOMMENT_LINE,
	tcCOMMENT_BLOCK,
	tcKEYWORD,
	tcSTRING,
	tcSTRING_QUOTE,
	tcCHARACTER,
	tcCHARACTER_QUOTE,
	tcESCAPE,
	tcPREPROC,
	tcNUMBER,
	tcHEX,
	tcOCT,
	tcPUNCTUATION,
	tcMD_HEADING,
	tcMD_ITALIC,
	tcMD_BOLD,
	tcMD_STRIKE,
	tcMD_VALUE,
	tcMD_CONETEXT,
	tcMD_BRACKET,
	tcMD_BRACKETPIC,
	tcMD_LINK,
	tcXML_ID,
	tcCSS_SELECTOR,
	tcCSS_PROPERTY,


	tcNUM_OF_TOKENS
};

#define MAX_COLORS 16

typedef struct femtoColor
{
	uint8_t r, g, b;

} femtoColor_t;

typedef struct femtoPalette
{
	const char * colorNames[MAX_COLORS];
	femtoColor_t colors[MAX_COLORS];
	femtoColor_t oldColors[MAX_COLORS];
	bool bUsePalette;

} femtoPalette_t;

typedef struct femtoSettings
{
	bool bHelpRequest:1;
	wchar * fileName;
	wchar * settingsFileName;

	bool bTabsToSpaces:1;
	uint8_t tabWidth;
	wchar * tabSpaceStr1;	// Contains space characters

	bool bAutoIndent:1;

	bool bWhiteSpaceVis:1;
	wchar whitespaceCh;
	WORD whitespaceCol;

	bool bRelLineNums:1;
	WORD lineNumCol;

	femtoPalette_t palette;
	WORD syntaxColors[tcNUM_OF_TOKENS];
	const char * syntaxTokens[tcNUM_OF_TOKENS];

	wchar lastErr[FEMTO_SETTINGS_ERR_MAX];

} femtoSettings_t;

/**
 * @brief Reset memory contents of a femtoSettings_t structure
 * 
 * @param self Pointer to femtoSettings_t structure
 */
void femtoSettings_reset(femtoSettings_t * restrict self);

/**
 * @brief Copy the last error message to an array, clear the initial array
 * 
 * @param self Pointer to femtoSettings_t structure
 * @param errArr Pointer to an array to copy error message to
 * @param errMax Maximum amount of characters to copy
 * @return true Getting last error succeeded
 * @return false No last error set
 */
bool femtoSettings_getLastError(femtoSettings_t * restrict self, wchar * restrict errArr, u32 errMax);

/**
 * @brief Generate tabs to spaces string, consisting of spaces to represent the tab
 * 
 * @param self Pointer to femtoSettings_t structure
 * @return true Success
 * @return false Failure
 */
bool femtoSettings_makeTabSpaceStr(femtoSettings_t * restrict self);

/**
 * @brief Populate settings using command line arguments
 * 
 * @param self Pointer to femtoSettings_t structure
 * @param argc Number of command line arguments
 * @param argv Command line argument vector
 * @return femtoErr_e femtoErr_ok -> everything is ok, femtoErr_unknown -> extra
 * information is necessary, get it via femtoSettings_getLastError()
 */
femtoErr_e femtoSettings_populate(femtoSettings_t * restrict self, int argc, const wchar ** restrict argv);

/**
 * @brief Try to load settings from configured file
 * 
 * @param self Pointer to femtoSettings_t structure
 * @return const wchar* Error message, NULL if everything is OK
 */
const wchar * femtoSettings_loadFromFile(femtoSettings_t * restrict self);

/**
 * @brief Destroy femtoSettings_t structure
 * 
 * @param self Pointer to femtoSettings_t structure
 */
void femtoSettings_destroy(femtoSettings_t * restrict self);


#endif

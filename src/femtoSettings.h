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
	tcCHARACTER,
	tcESCAPE,
	tcSTRING_QUOTE,
	tcCHARACTER_QUOTE,
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


	tcNUM_OF_TOKENS
};

typedef struct femtoSettings
{
	bool bHelpRequest:1;
	wchar_t * fileName;
	wchar_t * settingsFileName;

	bool bTabsToSpaces:1;
	uint8_t tabWidth;
	wchar_t * tabSpaceStr1;	// Contains space characters

	bool bAutoIndent:1;

	bool bWhiteSpaceVis:1;
	wchar_t whitespaceCh;
	WORD whitespaceCol;

	bool bRelLineNums:1;
	WORD lineNumCol;

	WORD syntaxColors[tcNUM_OF_TOKENS];

	wchar_t lastErr[FEMTO_SETTINGS_ERR_MAX];

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
bool femtoSettings_getLastError(femtoSettings_t * restrict self, wchar_t * restrict errArr, uint32_t errMax);

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
femtoErr_e femtoSettings_populate(femtoSettings_t * restrict self, int argc, const wchar_t ** restrict argv);

/**
 * @brief Try to load settings from configured file
 * 
 * @param self Pointer to femtoSettings_t structure
 * @return const wchar_t* Error message, NULL if everything is OK
 */
const wchar_t * femtoSettings_loadFromFile(femtoSettings_t * restrict self);

/**
 * @brief Destroy femtoSettings_t structure
 * 
 * @param self Pointer to femtoSettings_t structure
 */
void femtoSettings_destroy(femtoSettings_t * restrict self);


#endif

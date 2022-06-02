#ifndef FEMTOSETTINGS_H
#define FEMTOSETTINGS_H

#include "fCommon.h"

typedef enum fTokenColor
{
	tcTEXT,
	tcSEARCH_RESULT,
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

} fTokenColor_e, fTC_e;

#define MAX_COLORS 16

typedef struct fColor
{
	u8 r, g, b;

} fColor_t;

typedef struct fPalette
{
	const char * colorNames[MAX_COLORS];
	fColor_t colors[MAX_COLORS];
	fColor_t oldColors[MAX_COLORS];
	bool bUsePalette;

} fPalette_t;

typedef struct fSettings
{
	bool bHelpRequest:1;
	wchar * fileName;
	wchar * settingsFileName;

	bool bTabsToSpaces:1;
	u8 tabWidth;
	wchar * tabSpaceStr1;	// Contains space characters

	bool bAutoIndent:1;

	bool bWhiteSpaceVis:1;
	wchar whitespaceCh;
	WORD whitespaceCol;

	bool bRelLineNums:1;
	WORD lineNumCol;

	fPalette_t palette;
	WORD syntaxColors[tcNUM_OF_TOKENS];
	const char * syntaxTokens[tcNUM_OF_TOKENS];

	wchar lastErr[FEMTO_SETTINGS_ERR_MAX];

} fSettings_t;

/**
 * @brief Reset memory contents of a fSettings_t structure
 * 
 * @param self Pointer to fSettings_t structure
 */
void fSettings_reset(fSettings_t * restrict self);

/**
 * @brief Copy the last error message to an array, clear the initial array
 * 
 * @param self Pointer to fSettings_t structure
 * @param errArr Pointer to an array to copy error message to
 * @param errMax Maximum amount of characters to copy
 * @return true Getting last error succeeded
 * @return false No last error set
 */
bool fSettings_lastError(fSettings_t * restrict self, wchar * restrict errArr, usize errMax);

/**
 * @brief Generate tabs to spaces string, consisting of spaces to represent the tab
 * 
 * @param self Pointer to fSettings_t structure
 * @return true Success
 * @return false Failure
 */
bool fSettings_makeTabSpaceStr(fSettings_t * restrict self);

/**
 * @brief Populate settings using command line arguments
 * 
 * @param self Pointer to fSettings_t structure
 * @param argc Number of command line arguments
 * @param argv Command line argument vector
 * @return fErr_e ferr_ok -> everything is ok, ferr_unknown -> extra
 * information is necessary, get it via fSettings_lastError()
 */
fErr_e fSettings_cmdLine(fSettings_t * restrict self, int argc, const wchar ** restrict argv);

/**
 * @brief Try to load settings from configured file
 * 
 * @param self Pointer to fSettings_t structure
 * @return const wchar* Error message, NULL if everything is OK
 */
const wchar * fSettings_loadFromFile(fSettings_t * restrict self);

/**
 * @brief Destroy fSettings_t structure
 * 
 * @param self Pointer to fSettings_t structure
 */
void fSettings_destroy(fSettings_t * restrict self);


#endif

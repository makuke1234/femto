#ifndef FEMTOSETTINGS_H
#define FEMTOSETTINGS_H

#include "fCommon.h"
#include "fSyntax.h"

typedef struct fColor
{
	u8 r, g, b;

} fColor_t;

typedef struct fPalette
{
	const char * colorNames[MAX_CONSOLE_COLORS];
	fColor_t colors[MAX_CONSOLE_COLORS];
	fColor_t oldColors[MAX_CONSOLE_COLORS];
	bool bUsePalette;

} fPalette_t;

typedef struct fSettings
{
	wchar * fileName;
	wchar * settingsFileName;

	u8 tabWidth;
	wchar * tabSpaceStr1;	// Contains space characters


	wchar whitespaceCh;
	WORD whitespaceCol;

	bool bHelpRequest:1;
	bool bTabsToSpaces:1;
	bool bAutoIndent:1;
	bool bWhiteSpaceVis:1;
	bool bEnableLogging:1;
	
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

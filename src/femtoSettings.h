#ifndef FEMTOSETTINGS_H
#define FEMTOSETTINGS_H

#include "common.h"

#define FEMTO_SETTINGS_ERR_MAX 256

#define FEMTO_SETTINGS_FILE1 L"femto.json"
#define FEMTO_SETTINGS_FILE2 L"femto-settings.json"
#define FEMTO_SETTINGS_FILE3 L"femto_settings.json"
#define FEMTO_SETTINGS_FILE4 L"settings.json"


typedef struct femtoSettings
{
	bool helpRequested;
	wchar_t * fileName;
	wchar_t * settingsFileName;

	bool tabsToSpaces;
	uint8_t tabWidth;
	wchar_t * tabSpaceStr1;	// Contains space characters
	wchar_t * tabSpaceStr2;	// Contains tab characters

	bool insertTabs;


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
 * @return femtoErr_t femtoErr_ok -> everything is ok, femtoErr_unknown -> extra
 * information is necessary, get it via femtoSettings_getLastError()
 */
femtoErr_t femtoSettings_populate(femtoSettings_t * restrict self, int argc, const wchar_t ** restrict argv);

/**
 * @brief Try to load settings from configured file
 * 
 * @param self Pointer to femtoSettings_t structure
 * @return femtoErr_t femtoErr_ok -> everything is ok, femtoErr_unknown -> extra
 * information is necessary, get it via femtoSettings_getLastError()
 */
femtoErr_t femtoSettings_loadFromFile(femtoSettings_t * restrict self);

/**
 * @brief Destroy femtoSettings_t structure
 * 
 * @param self Pointer to femtoSettings_t structure
 */
void femtoSettings_destroy(femtoSettings_t * restrict self);


#endif

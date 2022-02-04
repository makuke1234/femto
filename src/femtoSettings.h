#ifndef FEMTOSETTINGS_H
#define FEMTOSETTINGS_H

#include "common.h"
#include "femto.h"

#define FEMTO_SETTINGS_ERR_MAX 256


typedef struct femtoSettings
{
	bool helpRequested;
	const wchar_t * fileName;


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
 * @brief Destroy femtoSettings_t structure
 * 
 * @param self Pointer to femtoSettings_t structure
 */
void femtoSettings_destroy(femtoSettings_t * restrict self);


#endif

#ifndef femtoDATA_H
#define femtoDATA_H

#include "common.h"
#include "femtoFile.h"
#include "femtoSettings.h"

typedef struct femtoDrawThread
{
	HANDLE hthread;

	CRITICAL_SECTION crit;
	CONDITION_VARIABLE cv;

	volatile bool bReady:1, bKillSwitch:1;

} femtoDrawThread_t;

typedef struct femtoData
{
	DWORD prevConsoleMode;
	bool bPrevConsoleModeSet;

	HANDLE conIn, conOut;
	struct
	{
		HANDLE handle;
		CHAR_INFO * mem;
		u32 w, h;
	} scrbuf;

	u32 filesSize, filesMax;
	femtoFile_t ** files;
	COORD * cursorpos;

	i32 fileIdx;

	femtoSettings_t settings;

	femtoDrawThread_t drawThread;

} femtoData_t;

/**
 * @brief Resets internal memory fields, zeroes them
 * 
 * @param self Pointer to femtoData_t structure
 */
bool femtoData_reset(femtoData_t * restrict self);
/**
 * @brief Initialises editor data strucutre, also sets atexit() handler function
 * 
 * @param self Pointer to femtoData_t structure
 * @return true Success
 * @return false Failure
 */
bool femtoData_init(femtoData_t * restrict self);
bool femtoData_loadPalette(femtoData_t * restrict self);
bool femtoData_restorePalette(const femtoData_t * restrict self);

/**
 * @brief Refreshes the screen's editing part only
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_refresh(femtoData_t * restrict self);
/**
 * @brief Multithreaded version of femtoData_refresh, refreshes the screen's editing
 * part only asynchronously
 * 
 * @param self Pointer to femtoData structure
 */
void femtoData_refreshThread(femtoData_t * restrict self);
/**
 * @brief Refreshes whole screen
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_refreshAll(femtoData_t * restrict self);
/**
 * @brief Draws a status bar message, refreshes statusbar
 * 
 * @param self Pointer to femtoData_t structure
 * @param message 
 */
void femtoData_statusDraw(femtoData_t * restrict self, const wchar * restrict message, const WORD * restrict colorData);
/**
 * @brief Refreshes status bar
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_statusRefresh(femtoData_t * restrict self);

/**
 * @brief Creates a new tab with desired fileName
 * 
 * @param self Pointer to femtoData_t structure
 * @param fileName File name
 * @return true Success
 * @return false Failure
 */
bool femtoData_openTab(femtoData_t * restrict self, const wchar * restrict fileName);
/**
 * @brief Closes the current active tab
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_closeTab(femtoData_t * restrict self);

/**
 * @brief Destroys editor's data structure, frees memory
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_destroy(femtoData_t * restrict self);

#endif

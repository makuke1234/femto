#ifndef femtoDATA_H
#define femtoDATA_H

#include "fCommon.h"
#include "fFile.h"
#include "fSettings.h"

typedef struct fDrawThreadData
{
	HANDLE hthread;

	CRITICAL_SECTION crit;
	CONDITION_VARIABLE cv;

	volatile bool bReady:1, bKillSwitch:1;

} fDrawThreadData_t;

typedef struct fData
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

	usize filesSize, filesMax;
	fFile_t ** files;
	COORD * cursorpos;

	isize fileIdx;

	fSettings_t settings;

	fDrawThreadData_t drawThread;

	wchar searchBuf[MAX_STATUS];
	const wchar * psearchTerm;
	bool bDirBack;

} fData_t;

/**
 * @brief Resets internal memory fields, zeroes them
 * 
 * @param self Pointer to fData_t structure
 */
bool fData_reset(fData_t * restrict self);
/**
 * @brief Initialises editor data strucutre, also sets atexit() handler function
 * 
 * @param self Pointer to fData_t structure
 * @return true Success
 * @return false Failure
 */
bool fData_init(fData_t * restrict self);
bool fData_loadPalette(fData_t * restrict self);
bool fData_restorePalette(const fData_t * restrict self);

/**
 * @brief Refreshes the screen's editing part only
 * 
 * @param self Pointer to fData_t structure
 */
void fData_refreshEdit(fData_t * restrict self);
/**
 * @brief Multithreaded version of fData_refreshEdit, refreshes the screen's editing
 * part only asynchronously
 * 
 * @param self Pointer to fData structure
 */
void fData_refreshEditAsync(fData_t * restrict self);
/**
 * @brief Refreshes whole screen
 * 
 * @param self Pointer to fData_t structure
 */
void fData_refreshAll(fData_t * restrict self);
/**
 * @brief Draws a status bar message, refreshes statusbar
 * 
 * @param self Pointer to fData_t structure
 * @param message 
 */
void fData_statusMsg(fData_t * restrict self, const wchar * restrict message, const WORD * restrict colorData);
/**
 * @brief Refreshes status bar
 * 
 * @param self Pointer to fData_t structure
 */
void fData_statusRefresh(fData_t * restrict self);

/**
 * @brief Creates a new tab with desired fileName
 * 
 * @param self Pointer to fData_t structure
 * @param fileName File name, can be NULL
 * @return true Success
 * @return false Failure
 */
bool fData_openTab(fData_t * restrict self, const wchar * restrict fileName);
/**
 * @brief Closes the current active tab
 * 
 * @param self Pointer to fData_t structure
 */
void fData_closeTab(fData_t * restrict self);

/**
 * @brief Destroys editor's data structure, frees memory
 * 
 * @param self Pointer to fData_t structure
 */
void fData_destroy(fData_t * restrict self);

#endif

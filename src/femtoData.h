#ifndef femtoDATA_H
#define femtoDATA_H

#include "common.h"
#include "femtoFile.h"


typedef struct femtoData_t
{
	HANDLE conIn, conOut;
	struct
	{
		HANDLE handle;
		wchar_t * mem;
		uint32_t w, h;
	} scrbuf;
	COORD cursorpos;

	femtoFile_t file;
} femtoData_t;

/**
 * @brief Resets internal memory fields, zeroes them
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_reset(femtoData_t * restrict self);
/**
 * @brief Initialises editor data strucutre, also sets atexit() handler function
 * 
 * @param self Pointer to femtoData_t structure
 * @return true Success
 * @return false Failure
 */
bool femtoData_init(femtoData_t * restrict self);
/**
 * @brief Refreshes the screen's editing part only
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_refresh(femtoData_t * restrict self);
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
void femtoData_statusDraw(femtoData_t * restrict self, const wchar_t * message);
/**
 * @brief Refreshes status bar
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_statusRefresh(femtoData_t * restrict self);

/**
 * @brief Destroys editor's data structure, frees memory
 * 
 * @param self Pointer to femtoData_t structure
 */
void femtoData_destroy(femtoData_t * restrict self);

#endif

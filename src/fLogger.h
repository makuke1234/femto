#ifndef FEMTO_LOGGER_H
#define FEMTO_LOGGER_H

#if LOGGING_ENABLE == 1

#include "fCommon.h"

/**
 * @brief Enables/disables the logger
 * 
 * @param enable Determines whether to enable or disable the logger
 */
void fLog_enable(bool enable);
/**
 * @brief Initialise profiler, exits on failure
 * 
 */
void fLog_init(void);
/**
 * @brief Closes profiler
 * 
 */
void fLog_close(void);
/**
 * @brief Write a profiler log message
 * 
 * @param function Function name of writer
 * @param format Standard printf message format
 * @param ... Variadic format arguments
 */
void fLog_write_inner(const char * restrict function, const char * restrict format, ...);
/**
 * @brief Start profiler timestamp
 * 
 */
void fLog_start(void);
/**
 * @brief Stop profiler timestamp
 * 
 * @param funcName Function name of timestamp writer
 */
void fLog_end_inner(const char * funcName);

#define fLog_write(...) fLog_write_inner(__func__, __VA_ARGS__)
#define fLog_end() fLog_end_inner(__func__)

#else

#define fLog_enable(enable)
#define fLog_init()
#define fLog_close()
#define fLog_write(...)
#define fLog_start()
#define fLog_end()

typedef int make_iso_compilers_happy;

#endif
#endif

#ifndef FEMTO_PROFILER_H
#define FEMTO_PROFILER_H

#if PROFILING_ENABLE == 1

#include "fCommon.h"

/**
 * @brief Initialise profiler, exits on failure
 * 
 */
void initProfiler(void);
/**
 * @brief Closes profiler
 * 
 */
void closeProfiler(void);
/**
 * @brief Write a profiler log message
 * 
 * @param function Function name of writer
 * @param format Standard printf message format
 * @param ... Variadic format arguments
 */
void writeProfiler_inner(const char * restrict function, const char * restrict format, ...);
/**
 * @brief Start profiler timestamp
 * 
 */
void profilerStart(void);
/**
 * @brief Stop profiler timestamp
 * 
 * @param funcName Function name of timestamp writer
 */
void profilerEnd_inner(const char * funcName);

#define writeProfiler(...) writeProfiler_inner(__func__, __VA_ARGS__)
#define profilerEnd() profilerEnd_inner(__func__)

#else

#define initProfiler()
#define closeProfiler()
#define writeProfiler(...)
#define profilerStart()
#define profilerEnd()

typedef int make_iso_compilers_happy;

#endif
#endif

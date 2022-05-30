#ifndef FEMTO_PROFILER_H
#define FEMTO_PROFILER_H

#if PROFILING_ENABLE == 1

#include "fCommon.h"

/**
 * @brief Initialise profiler, exits on failure
 * 
 */
void fProf_init(void);
/**
 * @brief Closes profiler
 * 
 */
void fProf_close(void);
/**
 * @brief Write a profiler log message
 * 
 * @param function Function name of writer
 * @param format Standard printf message format
 * @param ... Variadic format arguments
 */
void fProf_write_inner(const char * restrict function, const char * restrict format, ...);
/**
 * @brief Start profiler timestamp
 * 
 */
void fProf_start(void);
/**
 * @brief Stop profiler timestamp
 * 
 * @param funcName Function name of timestamp writer
 */
void fProf_end_inner(const char * funcName);

#define fProf_write(...) fProf_write_inner(__func__, __VA_ARGS__)
#define fProf_end() fProf_end_inner(__func__)

#else

#define fProf_init()
#define fProf_close()
#define fProf_write(...)
#define fProf_start()
#define fProf_end()

typedef int make_iso_compilers_happy;

#endif
#endif

#include "fProfiler.h"

#if PROFILING_ENABLE == 1

#include <time.h>
#include <stdarg.h>

static FILE * s_profilingFile = NULL;

void fProf_init(void)
{
	s_profilingFile = fopen(FEMTO_PROFILER_FILE, "a+");	
	if (s_profilingFile == NULL)
	{
		fputs("Error opening profiling file!\n", stderr);
		exit(1);
	}
	fputc('\n', s_profilingFile);
	fProf_write_inner("fProf_init", "Started application...");
}
void fProf_close(void)
{
	assert(s_profilingFile != NULL);
	
	fputc('\n', s_profilingFile);
	fProf_write_inner("fProf_close", "Closing profiler session...");
	
	// Closing file actually
	fclose(s_profilingFile);
	s_profilingFile = NULL;
}
void fProf_write_inner(const char * restrict function, const char * restrict format, ...)
{
	assert(function != NULL);
	assert(format != NULL);

	if (s_profilingFile == NULL)
	{
		fputs("Profiling file not open!\n", stderr);
		return;
	}

	// Write timestamp
	time_t rawtime;
	time(&rawtime);
	const struct tm * restrict ti = localtime(&rawtime);
	fprintf(
		s_profilingFile,
		"[%.2d.%.2d.%d @%.2d:%.2d:%.2d] @%s<",
		ti->tm_mday, ti->tm_mon + 1, ti->tm_year + 1900,
		ti->tm_hour, ti->tm_min,     ti->tm_sec,
		function
	);
	// Write message
	va_list ap;
	va_start(ap, format);

	vfprintf(s_profilingFile, format, ap);

	va_end(ap);

	fputs(">\n", s_profilingFile);
	fflush(s_profilingFile);
}

static clock_t s_profilerStack[FEMTO_PROFILER_STACK_SIZE];
static usize s_curStackLen = 0;

void fProf_start(void)
{
	assert(s_curStackLen < FEMTO_PROFILER_STACK_SIZE);
	s_profilerStack[s_curStackLen] = clock();
	++s_curStackLen;
}
void fProf_end_inner(const char * funcName)
{
	assert(funcName != NULL);
	assert(s_curStackLen > 0);
	
	--s_curStackLen;
	fProf_write_inner(
		funcName,
		"Elapsed %.3f s",
		(f64)(clock() - s_profilerStack[s_curStackLen]) / (f64)CLOCKS_PER_SEC
	);
}

#endif

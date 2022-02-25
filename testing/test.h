#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

static const char * lib = "";

static inline void setlib(const char * newlib)
{
	lib = newlib;
}

static inline void test(bool cond, const char * errmsg, ...)
{
	static int testnum = 0;
	
	++testnum;
	if (cond)
	{
		fprintf(stderr, "[%s] Test #%d passed\n", lib, testnum);
	}
	else
	{
		fprintf(stderr, "[%s] Test #%d failed: ", lib, testnum);
		va_list ap;
		va_start(ap, errmsg);
		vfprintf(stderr, errmsg, ap);
		va_end(ap);
		fprintf(stderr, "\n");
		exit(1);
	}
}

static inline void testn(const char * newlib, bool cond, const char * errmsg)
{
	setlib(newlib);
	test(cond, errmsg);
}

#endif

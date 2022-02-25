#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdbool.h>

static const char * lib = "";

static inline void setlib(const char * newlib)
{
	lib = newlib;
}

static inline void test(bool cond, const char * errmsg)
{
	static int testnum = 0;
	
	++testnum;
	if (cond)
	{
		fprintf(stderr, "[%s] Test #%d passed\n", lib, testnum);
	}
	else
	{
		fprintf(stderr, "[%s] Test #%d failed: \"%s\"\n", lib, testnum, errmsg);
	}
}

static inline void testn(const char * newlib, bool cond, const char * errmsg)
{
	setlib(newlib);
	test(cond, errmsg);
}

#endif

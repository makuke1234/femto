#include "safec.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

char * strdup_s(const char * restrict str, size_t len)
{
	if (str == NULL)
	{
		return NULL;
	}
	len = strnlen_s(str, len) + 1;
	char * mem = malloc(sizeof(char) * len);
	if (mem == NULL)
	{
		return NULL;
	}
	memcpy(mem, str, sizeof(char) * len);
	mem[len - 1] = '\0';
	return mem;
}
wchar_t * wcsdup_s(const wchar_t * restrict wstr, size_t len)
{
	if (wstr == NULL)
	{
		return NULL;
	}
	len = wcsnlen_s(wstr, len) + 1;
	wchar_t * mem = malloc(sizeof(wchar_t) * len);
	if (mem == NULL)
	{
		return NULL;
	}
	memcpy(mem, wstr, sizeof(wchar_t) * len);
	mem[len - 1] = L'\0';
	return mem;
}

char * strredup(char * restrict str, const char * restrict newstr)
{
	size_t exlen = (str != NULL) ? strlen(str) + 1 : 0;
	size_t newlen = strlen(newstr) + 1;
	if (newlen > exlen)
	{
		char * mem = realloc(str, sizeof(char) * newlen);
		if (mem == NULL)
		{
			if (str != NULL)
			{
				free(str);
			}
			return NULL;
		}
		str = mem;
	}
	memcpy(str, newstr, sizeof(char) * newlen);
	return str;
}
wchar_t * wcsredup(wchar_t * restrict str, const wchar_t * restrict newstr)
{
	size_t exlen = (str != NULL) ? wcslen(str) + 1 : 0;
	size_t newlen = wcslen(newstr) + 1;
	if (newlen > exlen)
	{
		wchar_t * mem = realloc(str, sizeof(wchar_t) * newlen);
		if (mem == NULL)
		{
			if (str != NULL)
			{
				free(str);
			}
			return NULL;
		}
		str = mem;
	}
	memcpy(str, newstr, sizeof(wchar_t) * newlen);
	return str;
}

char * dynstrcat_s(char ** restrict pstr, size_t * restrict psize, size_t strLen, const char * restrict addStr)
{
	assert(pstr   != NULL);
	assert(addStr != NULL);
	size_t strCap = 0;
	size_t * pstrCap = (psize != NULL) ? psize : &strCap;

	size_t addStrLen = strlen(addStr);
	size_t newLen = strLen + addStrLen + 1;
	if (newLen > *pstrCap)
	{
		size_t newCap = newLen * 2;
		char * mem = realloc(*pstr, sizeof(char) * newCap);
		if (mem == NULL)
		{
			return NULL;
		}
		*pstr    = mem;
		*pstrCap = newCap;
	}
	memcpy(&(*pstr)[strLen], addStr, sizeof(char) * addStrLen);
	(*pstr)[newLen - 1] = '\0';

	return *pstr;
}
char * dynstrncat_s(char ** restrict pstr, size_t * restrict psize, size_t strLen, const char * restrict addStr, size_t addStrLen)
{
	assert(pstr   != NULL);
	assert(addStr != NULL);
	size_t strCap = 0;
	size_t * pstrCap = (psize != NULL) ? psize : &strCap;

	size_t newLen = strLen + addStrLen + 1;
	if (newLen > *pstrCap)
	{
		size_t newCap = newLen * 2;
		char * mem = realloc(*pstr, sizeof(char) * newCap);
		if (mem == NULL)
		{
			return NULL;
		}
		*pstr    = mem;
		*pstrCap = newCap;
	}
	memcpy(&(*pstr)[strLen], addStr, sizeof(char) * addStrLen);
	(*pstr)[newLen - 1] = '\0';

	return *pstr;
}

wchar_t * dynwcscat_s(wchar_t ** restrict pstr, size_t * restrict psize, size_t strLen, const wchar_t * restrict addStr)
{
	assert(pstr   != NULL);
	assert(addStr != NULL);
	size_t strCap = 0;
	size_t * pstrCap = (psize != NULL) ? psize : &strCap;

	size_t addStrLen = wcslen(addStr);
	size_t newLen = strLen + addStrLen + 1;
	if (newLen > *pstrCap)
	{
		size_t newCap = newLen * 2;
		wchar_t * mem = realloc(*pstr, sizeof(wchar_t) * newCap);
		if (mem == NULL)
		{
			return NULL;
		}
		*pstr    = mem;
		*pstrCap = newCap;
	}
	memcpy(&(*pstr)[strLen], addStr, sizeof(wchar_t) * addStrLen);
	(*pstr)[newLen - 1] = L'\0';

	return *pstr;
}
wchar_t * dynwcsncat_s(wchar_t ** restrict pstr, size_t * restrict psize, size_t strLen, const wchar_t * restrict addStr, size_t addStrLen)
{
	assert(pstr   != NULL);
	assert(addStr != NULL);
	size_t strCap = 0;
	size_t * pstrCap = (psize != NULL) ? psize : &strCap;

	size_t newLen = strLen + addStrLen + 1;
	if (newLen > *pstrCap)
	{
		size_t newCap = newLen * 2;
		wchar_t * mem = realloc(*pstr, sizeof(wchar_t) * newCap);
		if (mem == NULL)
		{
			return NULL;
		}
		*pstr    = mem;
		*pstrCap = newCap;
	}
	memcpy(&(*pstr)[strLen], addStr, sizeof(wchar_t) * addStrLen);
	(*pstr)[newLen - 1] = L'\0';

	return *pstr;
}

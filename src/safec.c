#include "safec.h"

char * strdup_s(const char * str, size_t len)
{
	len = strnlen_s(str, len) + 1;
	char * mem = malloc(sizeof(char) * len);
	if (mem == NULL)
	{
		return NULL;
	}
	memcpy(mem, str, sizeof(char) * len);
	return mem;
}
wchar_t * wcsdup_s(const wchar_t * wstr, size_t len)
{
	len = wcsnlen_s(wstr, len) + 1;
	wchar_t * mem = malloc(sizeof(wchar_t) * len);
	if (mem == NULL)
	{
		return NULL;
	}
	memcpy(mem, wstr, sizeof(wchar_t) * len);
	return mem;
}

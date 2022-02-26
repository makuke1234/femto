#include "test.h"
#include "../src/jsonlite2/safec.h"

#include <string.h>
#include <stdint.h>

int wmain(void)
{
	setlib("strredup");

	char * str1 = strredup(NULL, "test1");
	test(str1 != NULL, "supposed not to be NULL");
	test(strcmp(str1, "test1") == 0, "supposed to be equal");
	char * str2 = strredup(str1, "test2");
	test(str2 != NULL, "supposed no to be NULL");
	test(str1 == str2, "supposed not to reallocate any memory");
	test(strcmp(str2, "test2") == 0, "supposed to be euqal");

	free(str2);

	return 0;
}

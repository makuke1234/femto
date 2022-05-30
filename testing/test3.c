#include "test.h"
#include "../src/jsonlite2/safec.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define TESTCAT(str1, str2) testcat(str1, str2, str1 str2)

static inline void testcat(const char * str1, const char * str2, const char * sum)
{
	char * receiver = strdup_s(str1, SIZE_MAX);
	test(strcmp(str1, receiver) == 0, "strings are not equal!");
	char * result = dynstrcat_s(&receiver, NULL, strlen(receiver), str2);
	test(result != NULL, "no result produced");
	test(strcmp(receiver, sum) == 0, "concatenated strings are not equal!");
	free(receiver);
}

int wmain(void)
{
	setlib("Safe C");

	test(strdup_s(NULL, 0) == NULL, "null string cannot be duplicated");
	char * str = strdup_s("", 0);
	test(str != NULL, "memory error!");
	test(strcmp(str, "") == 0, "duplicated string isn't empty");
	free(str);

	str = strdup_s("test", 0);
	test(strcmp(str, "") == 0, "string isn't empty after copying 0 characters");
	free(str);

	str = strdup_s("test", 1);
	test(strcmp(str, "t") == 0, "one character must be copied!");
	free(str);

	const char * testString = "aasdlfaksdlfhaklsdjfhalskdjfhalskdjfhaslkdjfh";
	for (usize i = 0, len = strlen(testString); i < len; ++i)
	{
		str = strdup_s(testString, i);
		test(strncmp(str, testString, i) == 0 && strncmp(str, testString, i + 1) < 0, "strings aren't equal!");
		free(str);
	}

	TESTCAT("asldkfjasldjh", "aa000188127837129873");
	TESTCAT("", "");
	TESTCAT("J", "");
	TESTCAT("AAA", "");
	TESTCAT("AAABBBCCC", "");
	TESTCAT("", "A");
	TESTCAT("", "AAABBBCCC");
	TESTCAT(" ", " ");
	TESTCAT("A", "A");
	TESTCAT("A", "B");
	TESTCAT("B", "A");


	return 0;
}

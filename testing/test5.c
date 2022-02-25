#include "test.h"
#include <jsonParser.h>

static inline void testCheck(const char * str)
{
	test(json_check(str, SIZE_MAX) == jsonErr_ok, "Checker should come clean: %s", str);
}
static inline void testCheckNot(const char * str)
{
	test(json_check(str, SIZE_MAX) != jsonErr_ok, "Checker should be angry: %s", str);
}

int wmain(void)
{
	setlib("JSON parser");

	testCheck("");
	testCheck("{}");
	testCheck("\"\"");
	testCheckNot("\"");
	testCheck("5");
	testCheck("null");
	testCheck("true");
	testCheck("false");
	testCheckNot("5,6");
	testCheckNot("{},{}");
	testCheck("[]");
	testCheck("[{},{},{},\"\"]");
	testCheckNot("{]");
	testCheckNot("[}");
	testCheckNot("{");
	testCheckNot("[");
	testCheckNot("{[");
	testCheckNot("{{");
	testCheckNot("[[");
	testCheckNot("[{");
	testCheckNot("]");
	testCheckNot("}");
	testCheckNot("nul");
	testCheckNot("tru");
	testCheckNot("fals");
	testCheckNot("abcdefg");
	testCheck("\"abcdefg\"");
	testCheckNot("[5, .5, 'single quotes', \"quotes in \"quotes\" in quotes\"]");
	testCheckNot("{ \"user\": \"<div class=\"user\">Ryan</div>\" }");
	testCheck("{ \"test\": \"each \n on \n new \n line\"}");

	return 0;
}

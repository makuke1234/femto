#include "test.h"
#include <jsonParser.h>
#include <math.h>

#define TEST_PARSE(pjson, str) test(json_parse(pjson, str, SIZE_MAX) == jsonErr_ok, "while parsing: %s", str)

int wmain(void)
{
	setlib("JSON parser");

	json_t json;
	bool suc;

	{
		TEST_PARSE(&json, "{}");

		const jsonObject_t * obj = jsonValue_getObject(&json.value, &suc);
		test(obj != NULL && suc, "Retrieving object failed!");

		json_destroy(&json);
	}

	{
		TEST_PARSE(&json, "[]");

		const jsonArray_t * arr = jsonValue_getArray(&json.value, &suc);
		test(arr != NULL && suc, "Retrieving array failed!");

		json_destroy(&json);
	}

	{
		TEST_PARSE(&json, "{\"key1\":5,\"key2\":null}");

		const jsonObject_t * obj = jsonValue_getObject(&json.value, &suc);
		test(obj != NULL && suc, "Retrieving object failed!");

		const jsonValue_t * val = jsonObject_get(obj, "key1");
		test(val != NULL, "Retrieving key1 failed!");

		f64 num = jsonValue_getNumber(val, &suc);
		test(num == 5.0 && suc, "Retrieving key's value failed!");

		val = jsonObject_get(obj, "key2");
		test(val != NULL, "Retrieving key2 failed!");

		jsonValue_getNull(val, &suc);
		test(suc, "Retrieving key's value failed!");

		val = jsonObject_get(obj, "key3");
		test(val == NULL, "That key shouldn't exist!");

		json_destroy(&json);
	}

	return 0;
}

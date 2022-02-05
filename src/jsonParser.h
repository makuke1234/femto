#ifndef JSONPARSER_H
#define JSONPARSER_H

#include "common.h"
#include "hashmap.h"

typedef hashMap_t jsonKeyMap_t;

enum jsonValueType
{
	jsonValue_null,
	jsonValue_string,
	jsonValue_boolean,
	jsonValue_number,
	jsonValue_array,
	jsonValue_object
};

typedef struct jsonArray
{
	size_t numValues, maxValues;
	jsonValue_t * values;
} jsonArray_t;

typedef struct jsonValue
{
	enum jsonValueType type;
	union
	{
		char * string;
		bool boolean;
		double number;
		jsonArray_t * array;
		jsonObject_t * object;
	} d;
} jsonValue_t;

typedef struct jsonKeyValue
{
	char * key;
	jsonValue_t value;
} jsonKeyValue_t;



typedef struct jsonObject
{
	size_t numKeys, maxKeys;
	jsonKeyValue_t * keyvalues;
	jsonKeyMap_t map;

} jsonObject_t;


typedef struct json
{
	jsonObject_t object;


} json_t;

#endif

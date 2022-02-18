#ifndef JSONPARSER_H
#define JSONPARSER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>
#include "hashmap.h"

#define MAX_NUMBERLEN 32

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

typedef enum jsonErr
{
	jsonErr_ok,
	jsonErr_unknown,
	jsonErr_mem,
	jsonErr_invalidChar,
	jsonErr_moreThan1Main,
	jsonErr_noKey,
	jsonErr_noTerminatingQuote,
	jsonErr_invalidValueSeparator,
	jsonErr_noValueSeparator,
	jsonErr_noValue,
	jsonErr_noSeparator,
	jsonErr_excessiveArrayTerminator,
	jsonErr_excessiveObjectTerminator,
	jsonErr_noArrayTerminator,
	jsonErr_noObjectTerminator,
	jsonErr_invalidTerminator,
	jsonErr_multipleDecimalPoints,
	jsonErr_noDigitAfterDecimal,
} jsonErr_t;

/**
 * @brief Array for error codes
 * 
 */
extern const char * g_jsonErrors[];

// Forward-declare jsonValue_t
typedef struct jsonValue jsonValue_t;

typedef struct jsonArray
{
	size_t numValues, maxValues;
	jsonValue_t * values;
} jsonArray_t;

void jsonArray_init(jsonArray_t * restrict self);
jsonArray_t * jsonArray_make(void);
void jsonArray_destroy(jsonArray_t * restrict self);
void jsonArray_free(jsonArray_t * restrict self);

jsonValue_t * jsonArray_push(jsonArray_t * restrict self);
bool jsonArray_pop(jsonArray_t * restrict self);

jsonErr_t jsonArray_dump(const jsonArray_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth);

// Forward-declare jsonObject_t
typedef struct jsonObject jsonObject_t;

struct jsonValue
{
	uint8_t type;
	union
	{
		char * string;
		bool boolean;
		double number;
		jsonArray_t * array;
		jsonObject_t * object;
	} d;
};

void jsonValue_init(jsonValue_t * restrict self);
void jsonValue_destroy(jsonValue_t * restrict self);

void   jsonValue_getNull   (const jsonValue_t * restrict self, bool * restrict success);
char * jsonValue_getString (const jsonValue_t * restrict self, bool * restrict success);
bool   jsonValue_getBoolean(const jsonValue_t * restrict self, bool * restrict success);
double jsonValue_getNumber (const jsonValue_t * restrict self, bool * restrict success);
jsonArray_t  * jsonValue_getArray (const jsonValue_t * restrict self, bool * restrict success);
jsonObject_t * jsonValue_getObject(const jsonValue_t * restrict self, bool * restrict success);

jsonErr_t jsonValue_dump(const jsonValue_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth, bool nonobj);

typedef struct jsonKeyValue
{
	char * key;
	jsonValue_t value;
} jsonKeyValue_t;

bool jsonKeyValue_init(jsonKeyValue_t * restrict self, const char * restrict key, jsonValue_t value);
bool jsonKeyValue_initRaw(jsonKeyValue_t * restrict self, char * restrict key, jsonValue_t value);
jsonKeyValue_t * jsonKeyValue_make(const char * restrict key, jsonValue_t value);
jsonKeyValue_t * jsonKeyValue_makeRaw(char * restrict key, jsonValue_t value);
void jsonKeyValue_destroy(jsonKeyValue_t * restrict self);
void jsonKeyValue_free(jsonKeyValue_t * restrict self);

jsonErr_t jsonKeyValue_dump(const jsonKeyValue_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth);

struct jsonObject
{
	size_t numKeys, maxKeys;
	jsonKeyValue_t ** keyvalues;
	jsonKeyMap_t map;
};

bool jsonObject_init(jsonObject_t * restrict self);
jsonObject_t * jsonObject_make(void);
void jsonObject_destroy(jsonObject_t * restrict self);
void jsonObject_free(jsonObject_t * restrict self);

bool jsonObject_exist(const jsonObject_t * restrict self, const char * restrict key);
bool jsonObject_insert(jsonObject_t * restrict self, const char * restrict key, jsonValue_t value);
bool jsonObject_insertRaw(jsonObject_t * restrict self, jsonKeyValue_t * restrict kv);
jsonValue_t * jsonObject_get(const jsonObject_t * restrict self, const char * restrict key);
bool jsonObject_remove(jsonObject_t * restrict self, const char * restrict key);

jsonErr_t jsonObject_dump(const jsonObject_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth);


typedef struct json
{
	jsonValue_t value;
} json_t;

void json_init(json_t * restrict self);
json_t * json_make(void);
void json_destroy(json_t * restrict self);
void json_free(json_t * restrict self);

jsonErr_t json_dump(const json_t * restrict self, char ** restrict cont, size_t * restrict contSize);

/**
 * @brief Check an UTF-8 encoded JSON file for any syntax errors
 *
 * @param contents UTF-8 encoded byte array of JSON file contents
 * @param contLen Contents length, given to strnlen_s to calculate actual length
 * @return jsonErr_t Error code, jsonErr_ok -> everything is ok
 */
jsonErr_t json_check(const char * restrict contents, size_t contLen);

/**
 * @brief Parse an UTF-8 encoded JSON file to an DOM tree, JSON will be checked first
 * by calling json_check()
 * 
 * @param self Pointer to json_t structure receiving the parsed tree
 * @param contents UTF-8 encoded byte array of JSON file contents
 * @param contLen Contents length, given to strnlen_s to calculate actual length
 * @return jsonErr_t Error code, jsonErr_ok -> everything is ok
 */
jsonErr_t json_parse(json_t * restrict self, const char * restrict contents, size_t contLen);


#ifdef __cplusplus
}
#endif

#endif

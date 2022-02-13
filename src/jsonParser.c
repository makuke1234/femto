#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

#include "jsonParser.h"
#include "safec.h"

#include <stdlib.h>
#include <assert.h>

const wchar_t * g_jsonErrors[] = {
	[jsonErr_ok]                        = L"All OK",
	[jsonErr_unknown]                   = L"Unknown error!",
	[jsonErr_mem]                       = L"Memory allocation error!",
	[jsonErr_invalidChar]               = L"Invalid character!",
	[jsonErr_moreThan1Main]             = L"More than 1 main value in file!",
	[jsonErr_noKey]                     = L"No key in object!",
	[jsonErr_noTerminatingQuote]        = L"No terminating quote for string!",
	[jsonErr_invalidValueSeparator]     = L"Invalid value separator in object!",
	[jsonErr_noValueSeparator]          = L"No value separator in object!",
	[jsonErr_noValue]                   = L"No value in object's key-value pair!",
	[jsonErr_noSeparator]               = L"No spearator between values!",
	[jsonErr_excessiveArrayTerminator]  = L"Excessive array terminator!",
	[jsonErr_excessiveObjectTerminator] = L"Excessive object terminator!",
	[jsonErr_noArrayTerminator]         = L"No terminating ']'!",
	[jsonErr_noObjectTerminator]        = L"No terminating '}'!",
	[jsonErr_invalidTerminator]         = L"Invalid terminator!",
	[jsonErr_multipleDecimalPoints]     = L"Multiple decimal points '.' in number!",
	[jsonErr_noDigitAfterDecimal]       = L"No digits after decimal point!",
};

void jsonArray_init(jsonArray_t * restrict self)
{
	assert(self != NULL);
	*self = (jsonArray_t){
		.numValues = 0,
		.maxValues = 0,
		.values    = NULL
	};
}
jsonArray_t * jsonArray_make(void)
{
	jsonArray_t * self = malloc(sizeof(jsonArray_t));
	if (self == NULL)
	{
		return NULL;
	}
	jsonArray_init(self);
	return self;
}
void jsonArray_destroy(jsonArray_t * restrict self)
{
	assert(self != NULL);
	if (self->values != NULL)
	{
		for (size_t i = 0; i < self->numValues; ++i)
		{
			jsonValue_destroy(&self->values[i]);
		}
		free(self->values);
		self->values = NULL;
	}
}
void jsonArray_free(jsonArray_t * restrict self)
{
	assert(self != NULL);
	jsonArray_destroy(self);
	free(self);
}

jsonValue_t * jsonArray_push(jsonArray_t * restrict self)
{
	if (self->numValues >= self->maxValues)
	{
		size_t newcap = (self->numValues + 1) * 2;
		jsonValue_t * mem = realloc(self->values, sizeof(jsonValue_t) * newcap);
		if (mem == NULL)
		{
			return NULL;
		}
		self->maxValues = newcap;
		self->values    = mem;
	}
	jsonValue_init(&self->values[self->numValues]);
	++self->numValues;
	return &self->values[self->numValues - 1];
}
bool jsonArray_pop(jsonArray_t * restrict self)
{
	if (self->numValues == 0)
	{
		return false;
	}
	--self->numValues;

	jsonValue_destroy(&self->values[self->numValues]);
	return true;
}

jsonErr_t jsonArray_dump(const jsonArray_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth)
{
	assert(self != NULL);
	assert(cont != NULL);
	size_t contLen = 0;
	size_t contCap = 0;
	size_t * prealCap = (contSize != NULL) ? contSize : &contCap;

	// Array start
	for (size_t i = 0; i < depth; ++i)
	{
		if (dynstrncat_s(cont, prealCap, contLen, "\t", 1) == NULL)
		{
			return jsonErr_mem;
		}
		++contLen;
	}
	if (dynstrncat_s(cont, prealCap, contLen, "[\n", 2) == NULL)
	{
		return jsonErr_mem;
	}
	contLen += 2;

	char * line = NULL;
	size_t lineCap = 0;
	for (size_t i = 0; i < self->numValues; ++i)
	{
		// Elements
		jsonErr_t result = jsonValue_dump(&self->values[i], &line, &lineCap, depth + 1);
		if (result != jsonErr_ok)
		{
			if (line != NULL)
			{
				free(line);
			}
			return result;
		}

		size_t lineLen = strlen(line);
		if (dynstrncat_s(cont, prealCap, contLen, line, lineLen) == NULL)
		{
			if (line != NULL)
			{
				free(line);
			}
			return jsonErr_mem;
		}
		contLen += lineLen;

		if (i < (self->numValues - 1))
		{
			if (dynstrncat_s(cont, prealCap, contLen - 1, ",\n", 2) == NULL)
			{
				if (line != NULL)
				{
					free(line);
				}
				return jsonErr_mem;
			}
			++contLen;
		}
	}
	if (line != NULL)
	{
		free(line);
	}

	// Array end
	for (size_t i = 0; i < depth; ++i)
	{
		if (dynstrncat_s(cont, prealCap, contLen, "\t", 1) == NULL)
		{
			return jsonErr_mem;
		}
		++contLen;
	}
	if (dynstrncat_s(cont, prealCap, contLen, "]\n", 2) == NULL)
	{
		return jsonErr_mem;
	}

	return jsonErr_ok;
}

void jsonValue_init(jsonValue_t * restrict self)
{
	assert(self != NULL);
	*self = (jsonValue_t){
		.type = jsonValue_null
	};
}
void jsonValue_destroy(jsonValue_t * restrict self)
{
	assert(self != NULL);
	assert((self->type >= jsonValue_null) && (self->type <= jsonValue_object));
	switch (self->type)
	{
	case jsonValue_string:
		if (self->d.string != NULL)
		{
			free(self->d.string);
			self->d.string = NULL;
		}
		break;
	case jsonValue_array:
		if (self->d.array != NULL)
		{
			jsonArray_destroy(self->d.array);
			free(self->d.array);
			self->d.array = NULL;
		}
		break;
	case jsonValue_object:
		if (self->d.object != NULL)
		{
			jsonObject_destroy(self->d.object);
			free(self->d.object);
			self->d.object = NULL;
		}
		break;
	default:
		// Do nothing
		break;
	}
}

void   jsonValue_getNull   (const jsonValue_t * restrict self, bool * restrict success)
{
	assert(self    != NULL);
	assert(success != NULL);
	*success = self->type == jsonValue_null;
}
char * jsonValue_getString (const jsonValue_t * restrict self, bool * restrict success)
{
	assert(self    != NULL);
	assert(success != NULL);
	*success = self->type == jsonValue_string;
	return self->d.string;
}
bool   jsonValue_getBoolean(const jsonValue_t * restrict self, bool * restrict success)
{
	assert(self    != NULL);
	assert(success != NULL);
	*success = self->type == jsonValue_boolean;
	return self->d.boolean;
}
double jsonValue_getNumber (const jsonValue_t * restrict self, bool * restrict success)
{
	assert(self    != NULL);
	assert(success != NULL);
	*success = self->type == jsonValue_number;
	return self->d.number;
}
jsonArray_t  * jsonValue_getArray (const jsonValue_t * restrict self, bool * restrict success)
{
	assert(self    != NULL);
	assert(success != NULL);
	*success = self->type == jsonValue_array;
	return self->d.array;
}
jsonObject_t * jsonValue_getObject(const jsonValue_t * restrict self, bool * restrict success)
{
	assert(self    != NULL);
	assert(success != NULL);
	*success = self->type == jsonValue_object;
	return self->d.object;
}

jsonErr_t jsonValue_dump(const jsonValue_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth)
{
	assert(self != NULL);
	assert(cont != NULL);
	size_t contCap = 0;
	size_t * prealCap = (contSize != NULL) ? contSize : &contCap;

	for (size_t i = 0; i < depth; ++i)
	{
		if (dynstrncat_s(cont, prealCap, i, "\t", 1) == NULL)
		{
			return jsonErr_mem;
		}
	}

	switch (self->type)
	{
	case jsonValue_null:
		if (dynstrncat_s(cont, prealCap, depth, "null\n", 5) == NULL)
		{
			return jsonErr_mem;
		}
		break;
	case jsonValue_string:
	{
		if (dynstrncat_s(cont, prealCap, depth, "\"", 1) == NULL)
		{
			return jsonErr_mem;
		}
		size_t len = strlen(self->d.string);
		if (dynstrncat_s(cont, prealCap, depth + 1, self->d.string, len) == NULL)
		{
			return jsonErr_mem;
		}
		if (dynstrncat_s(cont, prealCap, depth + 1 + len, "\"\n", 2) == NULL)
		{
			return jsonErr_mem;
		}
		break;
	}
	case jsonValue_boolean:
		if (dynstrncat_s(cont, prealCap, depth, self->d.boolean ? "true\n" : "false\n", self->d.boolean ? 5 : 6) == NULL)
		{
			return jsonErr_mem;
		}
		break;
	case jsonValue_number:
	{
		char temp[MAX_NUMBERLEN];
		int stringLen = sprintf_s(temp, MAX_NUMBERLEN, "%.15g\n", self->d.number);

		if ((stringLen <= 0) || dynstrncat_s(cont, prealCap, depth, temp, (size_t)stringLen) == NULL)
		{
			return jsonErr_mem;
		}
		break;
	}
	case jsonValue_array:
	{
		jsonErr_t ret = jsonArray_dump(self->d.array, cont, prealCap, depth);
		if (ret != jsonErr_ok)
		{
			return ret;
		}
		break;
	}
	case jsonValue_object:
	{
		jsonErr_t ret = jsonObject_dump(self->d.object, cont, prealCap, depth);
		if (ret != jsonErr_ok)
		{
			return ret;
		}
		break;
	}
	default:
		return jsonErr_unknown;
	}

	return jsonErr_ok;
}


bool jsonKeyValue_init(jsonKeyValue_t * restrict self, const char * restrict key, jsonValue_t value)
{
	assert(self != NULL);
	assert(key  != NULL);
	return jsonKeyValue_initRaw(self, strdup(key), value);
}
bool jsonKeyValue_initRaw(jsonKeyValue_t * restrict self, char * restrict key, jsonValue_t value)
{
	assert(self != NULL);
	assert(key  != NULL);

	self->key = key;
	if (self->key == NULL)
	{
		return false;
	}
	self->value = value;
	return true;
}
jsonKeyValue_t * jsonKeyValue_make(const char * restrict key, jsonValue_t value)
{
	assert(key != NULL);

	jsonKeyValue_t * self = malloc(sizeof(jsonKeyValue_t));
	if (self == NULL)
	{
		return NULL;
	}
	if (!jsonKeyValue_init(self, key, value))
	{
		free(self);
		return NULL;
	}
	return self;
}
jsonKeyValue_t * jsonKeyValue_makeRaw(char * restrict key, jsonValue_t value)
{
	assert(key != NULL);

	jsonKeyValue_t * self = malloc(sizeof(jsonKeyValue_t));
	if (self == NULL)
	{
		return NULL;
	}
	if (!jsonKeyValue_initRaw(self, key, value))
	{
		free(self);
		return NULL;
	}
	return self;
}
void jsonKeyValue_destroy(jsonKeyValue_t * restrict self)
{
	assert(self != NULL);
	if (self->key != NULL)
	{
		free(self->key);
		self->key = NULL;
	}
	jsonValue_destroy(&self->value);
}
void jsonKeyValue_free(jsonKeyValue_t * restrict self)
{
	assert(self != NULL);
	jsonKeyValue_destroy(self);
	free(self);
}

jsonErr_t jsonKeyValue_dump(const jsonKeyValue_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth)
{
	assert(self != NULL);
	assert(cont != NULL);
	size_t contLen = 0;
	size_t contCap = 0;
	size_t * prealCap = (contSize != NULL) ? contSize : &contCap;

	for (size_t i = 0; i < depth; ++i)
	{
		if (dynstrncat_s(cont, prealCap, contLen, "\t", 1) == NULL)
		{
			return jsonErr_mem;
		}
		++contLen;
	}
	if (dynstrncat_s(cont, prealCap, contLen, "\"", 1) == NULL)
	{
		return jsonErr_mem;
	}
	++contLen;

	size_t keyLen = strlen(self->key);
	if (dynstrncat_s(cont, prealCap, contLen, self->key, keyLen) == NULL)
	{
		return jsonErr_mem;
	}
	contLen += keyLen;

	if (dynstrncat_s(cont, prealCap, contLen, "\":\n", 3) == NULL)
	{
		return jsonErr_mem;
	}
	contLen += 3;


	char * line = NULL;
	jsonErr_t result = jsonValue_dump(&self->value, &line, NULL, depth);
	if (result != jsonErr_ok)
	{
		if (line != NULL)
		{
			free(line);
		}
		return jsonErr_mem;
	}

	if (dynstrcat_s(cont, prealCap, contLen, line) == NULL)
	{
		if (line != NULL)
		{
			free(line);
		}
		return jsonErr_mem;
	}

	free(line);

	return jsonErr_ok;
}

bool jsonObject_init(jsonObject_t * restrict self)
{
	assert(self != NULL);
	*self = (jsonObject_t){
		.numKeys   = 0,
		.maxKeys   = 0,
		.keyvalues = NULL
	};
	return hashMap_init(&self->map, 1);
}
jsonObject_t * jsonObject_make(void)
{
	jsonObject_t * self = malloc(sizeof(jsonObject_t));
	if (self == NULL)
	{
		return NULL;
	}
	if (!jsonObject_init(self))
	{
		free(self);
		return NULL;
	}
	return self;
}
void jsonObject_destroy(jsonObject_t * restrict self)
{
	assert(self != NULL);
	if (self->keyvalues != NULL)
	{
		for (size_t i = 0; i < self->numKeys; ++i)
		{
			jsonKeyValue_destroy(self->keyvalues[i]);
			free(self->keyvalues[i]);
		}
		free(self->keyvalues);
		self->keyvalues = NULL;
	}
	hashMap_destroy(&self->map);
}
void jsonObject_free(jsonObject_t * restrict self)
{
	assert(self != NULL);
	jsonObject_destroy(self);
	free(self);
}

bool jsonObject_exist(const jsonObject_t * restrict self, const char * restrict key)
{
	assert(self != NULL);
	assert(key  != NULL);
	return hashMap_get(&self->map, key) != NULL;
}
bool jsonObject_insert(jsonObject_t * restrict self, const char * restrict key, jsonValue_t value)
{
	assert(self != NULL);
	assert(key  != NULL);
	return jsonObject_insertRaw(self, &(jsonKeyValue_t){ .key = strdup(key), .value = value });
}
bool jsonObject_insertRaw(jsonObject_t * restrict self, jsonKeyValue_t * restrict kv)
{
	assert(self != NULL);
	assert(kv   != NULL);
	assert(kv->key != NULL);
	if (jsonObject_exist(self, kv->key))
	{
		return false;
	}

	writeProfiler("jsonObject_insertRaw", "key value: \"%s\"", kv->key);

	// Add to the end of array
	if (self->numKeys >= self->maxKeys)
	{
		size_t newSz = (self->numKeys + 1) * 2;
		jsonKeyValue_t ** mem = realloc(self->keyvalues, sizeof(jsonKeyValue_t *) * newSz);
		if (mem == NULL)
		{
			return false;
		}

		for (size_t i = self->maxKeys; i < newSz; ++i)
		{
			mem[i] = NULL;
		}
		self->keyvalues = mem;
		self->maxKeys   = newSz;
	}
	if (self->numKeys >= self->map.numNodes)
	{
		hashMap_resize(&self->map, (self->map.numNodes + 1) * 3);
	}

	self->keyvalues[self->numKeys] = jsonKeyValue_makeRaw(kv->key, kv->value);
	if (self->keyvalues[self->numKeys] == NULL)
	{
		return false;
	}
	else if (!hashMap_insert(&self->map, kv->key, &self->keyvalues[self->numKeys]->value))
	{
		free(self->keyvalues[self->numKeys]);
		return false;
	}

	++self->numKeys;
	return true;
}
jsonValue_t * jsonObject_get(const jsonObject_t * restrict self, const char * restrict key)
{
	assert(self != NULL);
	assert(key  != NULL);
	return hashMap_get(&self->map, key)->value;
}
bool jsonObject_remove(jsonObject_t * restrict self, const char * restrict key)
{
	assert(self != NULL);
	assert(key  != NULL);
	if (!hashMap_remove(&self->map, key) || (self->numKeys < 1))
	{
		return false;
	}

	// remove from array
	--self->numKeys;
	jsonKeyValue_destroy(self->keyvalues[self->numKeys]);
	free(self->keyvalues[self->numKeys]);
	self->keyvalues[self->numKeys] = NULL;

	return true;
}

jsonErr_t jsonObject_dump(const jsonObject_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth)
{
	assert(self != NULL);
	assert(cont != NULL);
	size_t contLen = 0;
	size_t contCap = 0;
	size_t * prealCap = (contSize != NULL) ? contSize : &contCap;

	// Object start
	for (size_t i = 0; i < depth; ++i)
	{
		if (dynstrncat_s(cont, prealCap, contLen, "\t", 1) == NULL)
		{
			return jsonErr_mem;
		}
		++contLen;
	}
	if (dynstrncat_s(cont, prealCap, contLen, "{\n", 2) == NULL)
	{
		return jsonErr_mem;
	}
	contLen += 2;

	char * line = NULL;
	size_t lineCap = 0;
	for (size_t i = 0; i < self->numKeys; ++i)
	{
		jsonErr_t result = jsonKeyValue_dump(self->keyvalues[i], &line, &lineCap, depth + 1);
		if (result != jsonErr_ok)
		{
			if (line != NULL)
			{
				free(line);
			}
			return result;
		}

		size_t lineLen = strlen(line);
		if (dynstrncat_s(cont, prealCap, contLen, line, lineLen) == NULL)
		{
			if (line != NULL)
			{
				free(line);
			}
			return jsonErr_mem;
		}
		contLen += lineLen;

		if (i < (self->numKeys - 1))
		{
			if (dynstrncat_s(cont, prealCap, contLen - 1, ",\n", 2) == NULL)
			{
				if (line != NULL)
				{
					free(line);
				}
				return jsonErr_mem;
			}
			++contLen;
		}
	}
	if (line != NULL)
	{
		free(line);
	}

	// Object end
	for (size_t i = 0; i < depth; ++i)
	{
		if (dynstrncat_s(cont, prealCap, contLen, "\t", 1) == NULL)
		{
			return jsonErr_mem;
		}
		++contLen;
	}
	if (dynstrncat_s(cont, prealCap, contLen, "}\n", 2) == NULL)
	{
		return jsonErr_mem;
	}

	return jsonErr_ok;
}

void json_init(json_t * restrict self)
{
	assert(self != NULL);
	jsonValue_init(&self->value);
}
json_t * json_make(void)
{
	json_t * json = malloc(sizeof(json_t));
	if (json == NULL)
	{
		return NULL;
	}
	json_init(json);
	return json;
}
void json_destroy(json_t * restrict self)
{
	assert(self != NULL);
	jsonValue_destroy(&self->value);
}
void json_free(json_t * restrict self)
{
	assert(self != NULL);
	json_destroy(self);
	free(self);
}

jsonErr_t json_dump(const json_t * restrict self, char ** restrict cont, size_t * restrict contSize)
{
	assert(self != NULL);
	assert(cont != NULL);
	return jsonValue_dump(&self->value, cont, contSize, 0);
}

static inline void json_inner_checkValue(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr);
static inline void json_inner_checkKeyValue(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr);
static inline size_t json_inner_checkValues(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr);
static inline void json_inner_checkObject(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr);
static inline void json_inner_checkArray(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr);


static inline void json_inner_checkValue(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr)
{
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);
	assert(perr != NULL);
	
	bool done = false;
	for (;*it != end; ++*it)
	{
		switch (**it)
		{
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			break;
		case ']':
		case '}':
			*perr = jsonErr_noValue;
			break;
		case '{':
			json_inner_checkObject(it, end, perr);
			done = true;
			break;
		case '[':
			json_inner_checkArray(it, end, perr);
			done = true;
			break;
		case '"':
			++*it;
			for (; *it != end; ++*it)
			{
				if ((**it == '\\') && ((*it + 1) != end))
				{
					++*it;
					continue;
				}
				else if (**it == '"')
				{
					done = true;
					++*it;
					break;
				}
			}
			if (*it == end)
			{
				*perr = jsonErr_noTerminatingQuote;
			}
			break;
		case 'f':
			if (((end - *it) >= 5) && (strncmp(*it, "false", 5) == 0))
			{
				*it += 5;
				done = true;
			}
			else
			{
				*perr = jsonErr_invalidChar;
			}
			break;
		case 't':
			if (((end - *it) >= 4) && (strncmp(*it, "true", 4) == 0))
			{
				*it += 4;
				done = true;
			}
			else
			{
				*perr = jsonErr_invalidChar;
			}
			break;
		case 'n':
			if (((end - *it) >= 4) && (strncmp(*it, "null", 4) == 0))
			{
				*it += 4;
				done = true;
			}
			else
			{
				*perr = jsonErr_invalidChar;
			}
			break;
		default:
			if ((**it >= '0') && (**it <= '9'))
			{
				done = true;

				bool dot = false;
				bool decAfterDot = false;

				++*it;
				for (; *it != end; ++*it)
				{
					if ((**it >= '0') && (**it <= '9'))
					{
						decAfterDot = dot;
						continue;
					}
					else if (**it == '.')
					{
						if (dot)
						{
							*perr = jsonErr_multipleDecimalPoints;
							break;
						}
						else
						{
							dot = true;
						}
					}
					else
					{
						break;
					}
				}
				if (dot && !decAfterDot)
				{
					*perr = jsonErr_noDigitAfterDecimal;
				}
			}
			else
			{
				*perr = jsonErr_invalidChar;
			}
		}
		if (done || (*perr != jsonErr_ok))
		{
			break;
		}
	}

	// Find for comma, ] or }
	for (; *it != end; ++*it)
	{
		if ((**it == ' ') || (**it == '\t') || (**it == '\n') || (**it == '\r'))
		{
			continue;
		}
		else if ((**it == ']') || (**it == '}'))
		{
			break;
		}
		else if (**it == ',')
		{
			++*it;
			break;
		}
		else
		{
			*perr = jsonErr_invalidTerminator;
			break;
		}
	}
}

static inline void json_inner_checkKeyValue(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr)
{
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);
	assert(perr != NULL);
	
	for (; *it != end; ++*it)
	{
		if (**it == '"')
		{
			break;
		}
		else if ((**it == ' ') || (**it == '\t') || (**it == '\n') || (**it == '\r'))
		{
			continue;
		}
		else
		{
			*perr = jsonErr_invalidChar;
			return;
		}
	}
	if (*it == end)
	{
		*perr = jsonErr_noKey;
		return;
	}

	++*it;
	for (; *it != end; ++*it)
	{
		if ((**it == '\\') && ((*it + 1) != end))
		{
			++*it;
			continue;
		}
		else if (**it == '"')
		{
			break;
		}
	}
	if (*it == end)
	{
		*perr = jsonErr_noTerminatingQuote;
		return;
	}

	++*it;
	for (; *it != end; ++*it)
	{
		if (**it == ':')
		{
			break;
		}
		switch (**it)
		{
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			break;
		default:
			*perr = jsonErr_invalidChar;
			return;
		}
	}

	if (*it == end)
	{
		*perr = jsonErr_noValueSeparator;
		return;
	}

	++*it;
	json_inner_checkValue(it, end, perr);
}

static inline size_t json_inner_checkValues(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr)
{
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);
	assert(perr != NULL);
	
	bool done = false;
	size_t vals = 0;
	while (*it != end)
	{
		switch (**it)
		{
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			++*it;
			break;
		case '}':
			*perr = jsonErr_invalidTerminator;
			break;
		case ']':
			done = true;
			break;
		default:
			json_inner_checkValue(it, end, perr);
			++vals;
		}
		if (done || (*perr != jsonErr_ok))
		{
			break;
		}
	}
	return vals;
}

static inline void json_inner_checkObject(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr)
{
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);
	assert(perr != NULL);
	
	for (; *it != end; ++*it)
	{
		if (**it == '{')
		{
			break;
		}
		else if ((**it == ' ') || (**it == '\t') || (**it == '\n') || (**it == '\r'))
		{
			continue;
		}
		else
		{
			*perr = jsonErr_invalidChar;
			return;
		}
	}
	if (*it == end)
	{
		*perr = jsonErr_noValue;
		return;
	}
	++*it;

	bool ended = false;
	while (*it != end)
	{
		switch (**it)
		{
		case '"':
			json_inner_checkKeyValue(it, end, perr);
			break;
		case '}':
			ended = true;
			/* fall through */
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			++*it;
			break;
		default:
			*perr = jsonErr_invalidChar;
		}

		if (ended || (*perr != jsonErr_ok))
		{
			break;
		}
	}
	if (!ended && (*perr == jsonErr_ok))
	{
		*perr = jsonErr_noObjectTerminator;
	}
}
static inline void json_inner_checkArray(const char * restrict * restrict it, const char * restrict end, jsonErr_t * restrict perr)
{
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);
	assert(perr != NULL);
	
	for (; *it != end; ++*it)
	{
		if (**it == '[')
		{
			break;
		}
		else if ((**it == ' ') || (**it == '\t') || (**it == '\n') || (**it == '\r'))
		{
			continue;
		}
		else
		{
			*perr = jsonErr_invalidChar;
			return;
		}
	}
	if (*it == end)
	{
		*perr = jsonErr_noValue;
		return;
	}
	++*it;

	bool ended = false;
	while (*it != end)
	{
		switch (**it)
		{
		case ']':
			ended = true;
			/* fall through */
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			++*it;
			break;
		default:
			json_inner_checkValues(it, end, perr);
		}

		if (ended || (*perr != jsonErr_ok))
		{
			break;
		}
	}
	if (!ended && (*perr == jsonErr_ok))
	{
		*perr = jsonErr_noArrayTerminator;
	}
}


jsonErr_t json_check(const char * restrict contents, size_t contLen)
{
	assert(contents != NULL);

	// Basically it's a state machine, stores a stack of states

	jsonErr_t err = jsonErr_ok;
	size_t firstLevelObjects = json_inner_checkValues(&contents, contents + strnlen_s(contents, contLen), &err);

	if ((err == jsonErr_ok) && (firstLevelObjects > 1))
	{
		err = jsonErr_moreThan1Main;
	}

	return err;
}

static inline jsonErr_t json_inner_parseValue(jsonValue_t * restrict self, const char * restrict * restrict it, const char * restrict end);
static inline jsonErr_t json_inner_parseKeyValue(jsonObject_t * restrict self, const char * restrict * restrict it, const char * restrict end);
static inline jsonErr_t json_inner_parseObject(jsonObject_t * restrict self, const char * restrict * restrict it, const char * restrict end);
static inline jsonErr_t json_inner_parseArray(jsonArray_t * restrict self, const char * restrict * restrict it, const char * restrict end);

static inline jsonErr_t json_inner_parseValue(jsonValue_t * restrict self, const char * restrict * restrict it, const char * restrict end)
{
	assert(self != NULL);
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);
	
	bool done = false;
	jsonErr_t err = jsonErr_ok;
	for (; *it != end; ++*it)
	{
		switch (**it)
		{
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			break;
		case '{':
			self->type = jsonValue_object;
			self->d.object = jsonObject_make();
			if (self->d.object == NULL)
			{
				return jsonErr_mem;
			}
			err = json_inner_parseObject(self->d.object, it, end);
			done = true;
			break;
		case '[':
			self->type = jsonValue_array;
			self->d.array = jsonArray_make();
			if (self->d.array == NULL)
			{
				return jsonErr_mem;
			}
			err = json_inner_parseArray(self->d.array, it, end);
			done = true;
			break;
		case '"':
			++*it;
			for (const char * begin = *it; *it != end; ++*it)
			{
				if ((**it == '\\') && ((*it + 1) != end))
				{
					++*it;
					continue;
				}
				else if (**it == '"')
				{
					char * str = strdup_s(begin, (size_t)(*it - begin));
					if (str == NULL)
					{
						return jsonErr_mem;
					}
					self->type = jsonValue_string;
					self->d.string = str;
					done = true;
					++*it;
					break;
				}
			}
			if (!done)
			{
				return jsonErr_unknown;
			}
			break;
		case 'f':
			if (((end - *it) >= 5) && (strncmp(*it, "false", 5) == 0))
			{
				*it += 5;
				self->type = jsonValue_boolean;
				self->d.boolean = false;
				done = true;
			}
			else
			{
				return jsonErr_unknown;
			}
			break;
		case 't':
			if (((end - *it) >= 4) && (strncmp(*it, "true", 4) == 0))
			{
				*it += 4;
				self->type = jsonValue_boolean;
				self->d.boolean = true;
				done = true;
			}
			else
			{
				return jsonErr_unknown;
			}
			break;
		case 'n':
			if (((end - *it) >= 4) && (strncmp(*it, "null", 4) == 0))
			{
				*it += 4;
				self->type = jsonValue_null;
				done = true;
			}
			else
			{
				return jsonErr_unknown;
			}
			break;
		default:
			if ((**it >= '0') && (**it <= '9'))
			{
				const char * begin = *it;

				for (; *it != end; ++*it)
				{
					if (((**it >= '0') && (**it <= '9')) || (**it == '.'))
					{
						continue;
					}
					else
					{
						break;
					}
				}
				self->type = jsonValue_number;
				self->d.number = strtod(begin, NULL);
				done = true;
			}
			else
			{
				return jsonErr_unknown;
			}
		}

		if (done || (err != jsonErr_ok))
		{
			break;
		}
	}
	if (err != jsonErr_ok)
	{
		return err;
	}

	// Find comma, ] or }
	for (; *it != end; ++*it)
	{
		if ((**it == ' ') || (**it == '\t') || (**it == '\n') || (**it == '\r'))
		{
			continue;
		}
		else if ((**it == ']') || (**it == '}'))
		{
			return jsonErr_ok;
		}
		else if (**it == ',')
		{
			++*it;
			return jsonErr_ok;
		}
		else
		{
			return jsonErr_unknown;
		}
	}
	return jsonErr_ok;
}
static inline jsonErr_t json_inner_parseKeyValue(jsonObject_t * restrict self, const char * restrict * restrict it, const char * restrict end)
{
	assert(self != NULL);
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);
	assert(**it == '"');

	jsonKeyValue_t kv;

	++*it;
	for (const char * begin = *it; *it != end; ++*it)
	{
		if ((**it == '\\') && ((*it + 1) != end))
		{
			++*it;
			continue;
		}
		else if (**it == '"')
		{
			char * key = strdup_s(begin, (size_t)(*it - begin));
			if (key == NULL)
			{
				return jsonErr_mem;
			}
			kv.key = key;
			++*it;
			break;
		}
	}

	for (; *it != end; ++*it)
	{
		if (**it == ':')
		{
			++*it;
			break;
		}
	}

	jsonErr_t err = json_inner_parseValue(&kv.value, it, end);
	if (err != jsonErr_ok)
	{
		return err;
	}
	if (!jsonObject_insertRaw(self, &kv))
	{
		return jsonErr_mem;
	}

	return jsonErr_ok;
}
static inline jsonErr_t json_inner_parseObject(jsonObject_t * restrict self, const char * restrict * restrict it, const char * restrict end)
{
	assert(self != NULL);
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);

	for (; *it != end; ++*it)
	{
		if (**it == '{')
		{
			break;
		}
	}
	++*it;

	bool ended = false;
	jsonErr_t err = jsonErr_ok;
	
	while (*it != end)
	{
		switch (**it)
		{
		case '"':
			err = json_inner_parseKeyValue(self, it, end);
			if (err != jsonErr_ok)
			{
				return err;
			}
			break;
		case '}':
			ended = true;
			/* fall through */
		case ']':
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			++*it;
			break;
		default:
			return jsonErr_unknown;
		}
		if (ended)
		{
			return jsonErr_ok;
		}
	}
	return jsonErr_unknown;
}
static inline jsonErr_t json_inner_parseArray(jsonArray_t * restrict self, const char * restrict * restrict it, const char * restrict end)
{
	assert(self != NULL);
	assert(it   != NULL);
	assert(*it  != NULL);
	assert(end  != NULL);

	for (; *it != end; ++*it)
	{
		if (**it == '[')
		{
			++*it;
			break;
		}
	}

	jsonErr_t err = jsonErr_ok;
	jsonValue_t * val = NULL;

	bool ended = false;
	while (*it != end)
	{
		switch (**it)
		{
		case ']':
			ended = true;
			/* fall through */
		case '}':
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			++*it;
			break;
		default:
			// Try to make value
			val = jsonArray_push(self);
			if (val == NULL)
			{
				return jsonErr_mem;
			}
			err = json_inner_parseValue(val, it, end);
			if (err != jsonErr_ok)
			{
				return err;
			}
		}
		if (ended)
		{
			return jsonErr_ok;
		}
	}
	return jsonErr_unknown;
}


jsonErr_t json_parse(json_t * restrict self, const char * restrict contents, size_t contLen)
{
	assert(self     != NULL);
	assert(contents != NULL);

	writeProfiler("json_parse", "checking JSON contents");
	jsonErr_t err = json_check(contents, contLen);
	if (err != jsonErr_ok)
	{
		return err;
	}
	// err == jsonErr_ok, if code has reached here
	
	writeProfiler("json_parse", "parsing JSON contents");
	return json_inner_parseValue(&self->value, &contents, contents + strnlen_s(contents, contLen));
}


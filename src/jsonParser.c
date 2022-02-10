#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>

#include "jsonParser.h"
#include "stack.h"
#include "safec.h"

#include <stdlib.h>
#include <assert.h>

const char * g_jsonErrors[] = {
	[jsonErr_ok]                        = "All OK",
	[jsonErr_unknown]                   = "Unknown error!",
	[jsonErr_mem]                       = "Memory allocation error!",
	[jsonErr_invalidChar]               = "Invalid character!",
	[jsonErr_invalidState]              = "Invalid state!",
	[jsonErr_moreThan1Object]           = "More than 1 object in file!",
	[jsonErr_noKey]                     = "No key in object!",
	[jsonErr_noTerminatingQuote]        = "No terminating quote for string!",
	[jsonErr_invalidValueSeparator]     = "Invalid value separator in object!",
	[jsonErr_noValueSeparator]          = "No value separator in object!",
	[jsonErr_noValue]                   = "No value in object's key-value pair!",
	[jsonErr_noSeparator]               = "No spearator between values!",
	[jsonErr_excessiveArrayTerminator]  = "Excessive array terminator!",
	[jsonErr_excessiveObjectTerminator] = "Excessive object terminator!",
	[jsonErr_noArrayTerminator]         = "No terminating ']'!",
	[jsonErr_noObjectTerminator]        = "No terminating '}'!",
	[jsonErr_invalidTerminator]         = "Invalid terminator!",
	[jsonErr_multipleDecimalPoints]     = "Multiple decimal points '.' in number!"
};

static inline bool stack_pushCh(stack_t * restrict self, char ch)
{
	return stack_push(self, &ch);
}
static inline char stack_topCh(const stack_t * restrict self)
{
	char * c = stack_top(self);
	if (c == NULL)
	{
		return 0;
	}
	else
	{
		return *c;
	}
}

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
		jsonErr_t ret = jsonArray_dump(self->d.array, cont, prealCap, depth + 1);
		if (ret != jsonErr_ok)
		{
			return ret;
		}
		break;
	}
	case jsonValue_object:
	{
		jsonErr_t ret = jsonObject_dump(self->d.object, cont, prealCap, depth + 1);
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
	
	self->key = strdup(key);
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
	if (dynstrncat_s(cont, prealCap, contLen, ":\n", 2) == NULL)
	{
		return jsonErr_mem;
	}
	contLen += 2;

	size_t keyLen = strlen(self->key);
	if (dynstrncat_s(cont, prealCap, contLen, self->key, keyLen) == NULL)
	{
		return jsonErr_mem;
	}

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
	if (jsonObject_exist(self, key))
	{
		return false;
	}

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

	self->keyvalues[self->numKeys] = jsonKeyValue_make(key, value);
	if (self->keyvalues[self->numKeys] == NULL)
	{
		return false;
	}
	else if (!hashMap_insert(&self->map, key, &self->keyvalues[self->numKeys]->value))
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

bool json_init(json_t * restrict self)
{
	assert(self != NULL);
	return jsonObject_init(&self->object);
}
json_t * json_make(void)
{
	json_t * json = malloc(sizeof(json_t));
	if (json == NULL)
	{
		return NULL;
	}
	if (!json_init(json))
	{
		free(json);
		return NULL;
	}
	return json;
}
void json_destroy(json_t * restrict self)
{
	assert(self != NULL);
	jsonObject_destroy(&self->object);
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
	return jsonObject_dump(&self->object, cont, contSize, 0);
}


static inline jsonErr_t json_inner_checkValue(const char ** restrict pp, const char * restrict endp, stack_t * restrict states)
{
	assert(pp     != NULL);
	assert(*pp    != NULL);
	assert(endp   != NULL);
	assert(states != NULL);
	char objStart = stack_topCh(states);
	assert(objStart != 0);

	jsonErr_t err = jsonErr_ok;

	const char * p = *pp;
	
	bool value = false;
	// Scan 1 value, then scan for comma or object/array end depending on mode
	for (; p != endp; ++p)
	{
		bool done = false;
		switch (*p)
		{
		// Skips whitespace
		case ' ':
		case '\t':
		case '\n':
		case '\r':
			break;
		case '{':
		case '[':
			--p;
			done = true;
			break;
		case '"':
			done = true;
			++p;
			for (; p != endp; ++p)
			{
				if ((*p == '\\') && ((p + 1) != endp))
				{
					++p;
					continue;
				}
				else if (*p == '"')
				{
					break;
				}
			}
			if (p == endp)
			{
				err = jsonErr_noTerminatingQuote;
				break;
			}
			// If value found
			value = true;
			break;
		default:
			// Check for true, false, null
			done = true;
			value = true;
			if (((endp - p) >= 5) && (strncmp(p, "false", 5) == 0))
			{
				p += 4;
			}
			else if (((endp - p) >= 4) && (strncmp(p, "true", 4) == 0))
			{
				p += 3;
			}
			else if (((endp - p) >= 4) && (strncmp(p, "null", 4) == 0))
			{
				p += 3;
			}
			else if ((*p >= '0') && (*p <= '9'))
			{
				// Iterate to number end
				bool dot = false;
				
				++p;
				for (; p != endp; ++p)
				{
					if ((*p >= '0') && (*p <= '9'))
					{
						continue;
					}
					else if (*p == '.')
					{
						if (dot)
						{
							err   = jsonErr_multipleDecimalPoints;
							value = false;
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
			}
			else
			{
				value = false;
			}
		}
		if (done || (err != jsonErr_ok))
		{
			break;
		}
	}

	if (value)
	{
		// try to find for end or comma
		for (; p != endp; ++p)
		{
			bool done = false;
			switch (*p)
			{
			// Skip whitespace
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				break;
			case ']':
				--p;
				done = true;
				if (objStart != '[')
				{
					err = jsonErr_invalidTerminator;
				}
				break;
			case '}':
				--p;
				done = true;
				if (objStart != '{')
				{
					err = jsonErr_invalidTerminator;
				}
				break;
			case ',':
				done = true;
				break;
			}

			if (done)
			{
				break;
			}
		}
	}
	else if (err == jsonErr_ok)
	{
		err = jsonErr_noValue;
	}

	*pp = p;

	return err;
}


jsonErr_t json_check(const char * restrict contents, size_t contLen)
{
	assert(contents != NULL);

	contLen = strnlen_s(contents, contLen);

	// Basically it's a state machine, stores a stack of states

	stack_t states;
	stack_init(&states, sizeof(char));

	size_t firstLevelObjects = 0;

	jsonErr_t err = jsonErr_ok;

	const char * endp = contents + contLen;
	for (const char * p = contents; p != endp; ++p)
	{
		if (states.len > 0)
		{
			switch (stack_topCh(&states))
			{
			case '{':
				switch (*p)
				{
				// Skips whitespace
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					break;
				case '}':
					if (!stack_pop(&states))
					{
						err = jsonErr_excessiveObjectTerminator;
					}
					break;
				case '"':
					// Parse key until end
					++p;
					for (; p != endp; ++p)
					{
						if ((*p == '\\') && ((p + 1) != endp))
						{
							++p;
							continue;
						}
						else if (*p == '"')
						{
							break;
						}
					}
					if (p == endp)
					{
						err = jsonErr_noTerminatingQuote;
						break;
					}
					++p;
					for (; p != endp; ++p)
					{
						// skip whitespace
						if ((*p == ' ') || (*p == '\t') || (*p == '\n') || (*p == '\r'))
						{
							continue;
						}
						else if (*p == ':')
						{
							break;
						}
						else
						{
							err = jsonErr_invalidValueSeparator;
							break;
						}
					}
					if (err != jsonErr_ok)
					{
						break;
					}
					else if (p == endp)
					{
						err = jsonErr_noValueSeparator;
						break;
					}
					++p;
					
					err = json_inner_checkValue(&p, endp, &states);
					if (err != jsonErr_ok)
					{
						break;
					}

					break;
				default:
					err = jsonErr_noKey;
				}
				break;
			case '[':
				switch (*p)
				{
				// Skips whitespace
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					break;
				case ']':
					if (!stack_pop(&states))
					{
						err = jsonErr_excessiveArrayTerminator;
					}
					break;
				default:
					err = json_inner_checkValue(&p, endp, &states);
				}
				break;
			default:
				err = jsonErr_invalidState;
			}
			if (err != jsonErr_ok)
			{
				break;
			}
		}
		else
		{
			switch (*p)
			{
			// Skips whitespace
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				break;
			case '{':
				++firstLevelObjects;
				if (!stack_pushCh(&states, '{'))
				{
					err = jsonErr_mem;
				}
				break;
			default:
				err = jsonErr_invalidChar;
			}

			if (err != jsonErr_ok)
			{
				break;
			}
		}
	}

	if ((states.len > 0) && (err == jsonErr_ok))
	{
		char ch = stack_topCh(&states);
		if (ch == '{')
		{
			err = jsonErr_noObjectTerminator;
		}
		else if (ch == '[')
		{
			err = jsonErr_noArrayTerminator;
		}
		else
		{
			err = jsonErr_unknown;
		}
	}

	// free stack
	stack_destroy(&states);

	if ((firstLevelObjects > 1) && (err == jsonErr_ok))
	{
		// Too much objects
		err = jsonErr_moreThan1Object;
	}

	return err;
}

jsonErr_t json_parse(json_t * restrict self, const char * restrict contents, size_t contLen)
{
	assert(self     != NULL);
	assert(contents != NULL);

	jsonErr_t err = json_check(contents, contLen);
	if (err != jsonErr_ok)
	{
		return err;
	}

	contLen = strnlen_s(contents, contLen);


	return jsonErr_ok;
}

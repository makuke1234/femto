#include "jsonParser.h"

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
	char * str = *cont;
	size_t realSize = (contSize != NULL) ? *contSize : 0;

	// Array start

	for (size_t i = 0; i < self->numValues; ++i)
	{
		// Elements
	}

	// Array end

	*cont = str;
	if (contSize != NULL)
	{
		*contSize = realSize;
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
	size_t realSize = (contSize != NULL) ? *contSize : 0;

	char temp[MAX_NUMBERLEN];
	char * string    = NULL;
	size_t stringLen = 0;
	bool needsDealloc = false;

	switch (self->type)
	{
	case jsonValue_null:
		string    = "null";
		stringLen = 5;
		break;
	case jsonValue_string:
	{
		size_t size = strlen(self->d.string);
		size_t totalSize = 1 + size + 1 + 1;
		string = malloc(sizeof(char) * totalSize);
		if (string == NULL)
		{
			return jsonErr_mem;
		}

		string[0] = '"';
		memcpy(&string[1], self->d.string, sizeof(char) * size);
		string[totalSize - 2] = '"';
		string[totalSize - 1] = '\0';

		stringLen = totalSize;

		needsDealloc = true;
		break;
	}
	case jsonValue_boolean:
		string    = self->d.boolean ? "true" : "false";
		stringLen = self->d.boolean ? 5      : 6;
		break;
	case jsonValue_number:
		string    = temp;
		stringLen = sprintf_s(temp, MAX_NUMBERLEN, "%.15g", self->d.number) + 1;
		break;
	case jsonValue_array:
	{
		jsonErr_t ret = jsonArray_dump(self->d.array, &string, &stringLen, depth + 1);
		if (ret != jsonErr_ok)
		{
			return ret;
		}
		needsDealloc = true;
		break;
	}
	case jsonValue_object:
	{
		jsonErr_t ret = jsonObject_dump(self->d.object, &string, &stringLen, depth + 1);
		if (ret != jsonErr_ok)
		{
			return ret;
		}
		needsDealloc = true;
		break;
	}
	}

	if (string == NULL)
	{
		return jsonErr_unknown;
	}

	// Copy string contents

	if (realSize < stringLen)
	{
		char * str = realloc(*cont, sizeof(char) * stringLen);
		if (str == NULL)
		{
			if (needsDealloc)
			{
				free(string);
			}
			return jsonErr_mem;
		}
		realSize = stringLen;
	}
	memcpy(*cont, string, sizeof(char) * stringLen);


	if (needsDealloc)
	{
		free(string);
	}

	if (contSize != NULL)
	{
		*contSize = realSize;
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
	char * str = *cont;
	size_t realSize = (contSize != NULL) ? *contSize : 0;



	*cont = str;
	if (contSize != NULL)
	{
		*contSize = realSize;
	}
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

jsonErr_t jsonObject_dump(const json_t * restrict self, char ** restrict cont, size_t * restrict contSize, size_t depth)
{
	assert(self != NULL);
	assert(cont != NULL);
	char * str = *cont;
	size_t realSize = (contSize != NULL) ? *contSize : 0;



	*cont = str;
	if (contSize != NULL)
	{
		*contSize = realSize;
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

jsonErr_t json_parse(json_t * restrict self, const char * contents, size_t contLen)
{
	assert(self     != NULL);
	assert(contents != NULL);

	size_t realLen = strnlen_s(contents, contLen);

	return jsonErr_ok;
}


jsonErr_t json_dump(const json_t * restrict self, char ** restrict cont, size_t * restrict contSize)
{
	assert(self != NULL);
	assert(cont != NULL);
	return jsonObject_dump(&self->object, cont, contSize, 0);
}

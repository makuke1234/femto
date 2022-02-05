#include "jsonParser.h"

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



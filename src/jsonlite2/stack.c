#include "stack.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void stack_init(stack_t * restrict self, size_t elemSize)
{
	assert(self != NULL);

	*self = (stack_t){
		.mem      = NULL,
		.elemSize = elemSize,
		.len      = 0,
		.cap      = 0
	};
}
void stack_destroy(stack_t * restrict self)
{
	assert(self != NULL);

	if (self->mem != NULL)
	{
		free(self->mem);
		self->mem = NULL;
	}
}

bool stack_push(stack_t * restrict self, const void * restrict pitem)
{
	assert(self  != NULL);
	assert(pitem != NULL);

	if (self->len >= self->cap)
	{
		size_t newcap = (self->len + 1) * 2;
		void * mem = realloc(self->mem, self->elemSize * newcap);
		if (mem == NULL)
		{
			return false;
		}

		self->mem = mem;
		self->cap = newcap;
	}

	memcpy((uint8_t *)self->mem + self->elemSize * self->len, pitem, self->elemSize);
	++self->len;

	return true;
}
bool stack_pop(stack_t * restrict self)
{
	assert(self != NULL);

	if (self->len == 0)
	{
		return false;
	}
	--self->len;

	return true;
}
void * stack_top(const stack_t * restrict self)
{
	assert(self != NULL);

	if (self->len == 0)
	{
		return NULL;
	}

	return (uint8_t *)self->mem + self->elemSize * (self->len - 1);
}


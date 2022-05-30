#include "test.h"
#include "../src/jsonlite2/stack.h"

#include <stdint.h>

int wmain(void)
{
	setlib("Stack");

	stack_t st;
	stack_init(&st, sizeof(uint8_t));

	test(stack_push(&st, &(uint8_t){ 255 }), "pushing to stack failed");
	test(st.len > 0, "stack is empty");
	uint8_t * top = stack_top(&st);
	test(top != NULL, "stack didn't return top");
	test(*top == 255, "stack returned invalid value!");

	test(stack_pop(&st), "popping from stack failed");
	test(!stack_pop(&st), "still items to pop");
	test(st.len == 0, "stack is not empty");

	for (usize i = 0; i < 256; ++i)
	{
		test(stack_push(&st, &(uint8_t){ (uint8_t)i }), "pushing failed");
		test(stack_top(&st) != NULL, "stack top is empty");
		test(*(uint8_t *)stack_top(&st) == i, "stack has different value than was pushed");
	}

	for (usize i = 0; i < 256; ++i)
	{
		uint8_t expval = (uint8_t)(255 - i);
		test(stack_top(&st) != NULL, "stack top is already empty");
		test(*(uint8_t *)stack_top(&st) == expval, "stack has different value than was pushed");
		test(stack_pop(&st), "stack cannot be empty");
	}

	test(st.len == 0, "stack cannot contain something at this point");

	stack_destroy(&st);

	return 0;
}

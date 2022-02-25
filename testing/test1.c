#include "test.h"

int wmain(void)
{
	testn("Always true", true, "That can't be false!");
	testn("Always false", false, "This is always false!");

	return 0;
}

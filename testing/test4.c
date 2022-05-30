#include "test.h"
#include "../src/jsonlite2/hashmap.h"

#include <string.h>

static inline void testInsert(hashMap_t * map, const char * key, intptr_t value, usize newsize)
{
	hashMap_insert(map, key, (vptr)value);
	test(hashMap_get(map, key) != NULL, "insertion failed");

	if (newsize)
	{
		hashMap_resize(map, newsize);
	}
}

static inline void testRemove(hashMap_t * map, const char * key, intptr_t value)
{
	hashNode_t * node = hashMap_get(map, key);
	test(node != NULL, "that key should exist!");
	test(strcmp(node->key, key) == 0, "keys don't match");
	test(node->value == (vptr)value, "value doesn't match");
	test(hashMap_remove(map, key), "removing element failed");
	test(!hashMap_remove(map, key), "removing element again didn't fail");
}

int wmain(void)
{
	setlib("Hash-map");

	hashMap_t map;
	test(hashMap_init(&map, 1), "hash-map initialisation failed");
	
	test(hashMap_resize(&map, 100), "hash-map resizing failed!");

	testInsert(&map, "asdfasdfasdfhlkjadhgjjajshdfj", 0xDEFE125A, 250);
	testInsert(&map, "asdfasdfasdfhlkjadhgjjajshdfj", 0xDEABCD5A, 2);
	testInsert(&map, "asdfasdfasdfhlkjadhgjjajshzxllkdfj", 0xD55678A5, 60000);
	testInsert(&map, "asdfasdfasdsdfgdfj", 0xDEF98765, 120);

	test(hashMap_resize(&map, 100), "hash-map resizing failed!");
	test(hashMap_resize(&map, 100000), "hash-map resizing failed!");
	test(hashMap_resize(&map, 1000), "hash-map resizing failed!");
	test(hashMap_resize(&map, 1), "hash-map resizing failed!");

	testRemove(&map, "asdfasdfasdfhlkjadhgjjajshdfj", 0xDEFE125A);
	testRemove(&map, "asdfasdfasdfhlkjadhgjjajshzxllkdfj", 0xD55678A5);
	testRemove(&map, "asdfasdfasdsdfgdfj", 0xDEF98765);


	hashMap_destroy(&map);

	return 0;
}

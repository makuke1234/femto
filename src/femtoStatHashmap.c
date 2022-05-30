#include "femtoStatHashmap.h"

#include <hashmap.h>

bool fHash_isPrime(u32 number)
{
	if (number <= 3)
	{
		return number > 1;
	}
	else
	{
		for (u32 i = 3, n = (u32)sqrtf((f32)number) + 1; i < n; i += 2)
		{
			if ((number % i) == 0)
			{
				return false;
			}
		}
		return true;
	}
}
u32 fHash_findNextPrime(u32 lowerBound)
{
	assert(lowerBound > 0);

	lowerBound += (lowerBound > 2) && ((lowerBound % 2) == 0);

	while (!fHash_isPrime(lowerBound))
	{
		lowerBound += 2;
	}
	return lowerBound;
}

u32 fHash_hash(const wchar * restrict key, u32 sz)
{
	assert(key != NULL);
	assert(sz > 0);

	u32 hash = 0;
	for (; (*key) != L'\0'; ++key)
	{
		hash = (u32)(hash * 37 + (u32)*key);
	}
	return hash % sz;
}

vptr fHash_malloc(fHash_t * restrict hash, u32 bytes)
{
	if (hash->memSize < bytes)
	{
		return NULL;
	}

	vptr mem = hash->mem;
	hash->mem = ((uint8_t *)hash->mem) + bytes;
	hash->memSize -= bytes;

	return mem;
}
void fHash_free(fHash_t * restrict hash, vptr restrict mem, u32 bytes)
{
	vptr newp = ((uint8_t *)hash->mem) - bytes;
	if (mem == newp)
	{
		hash->mem = newp;
		hash->memSize += bytes;
	}
}

void fHash_init(
	fHash_t * restrict hash,
	vptr restrict memory, u32 memoryBytes,
	u32 tableSize
)
{
	assert(hash != NULL);
	assert(memory != NULL);
	assert(memoryBytes > 0);
	assert(tableSize > 0);

	if (hash->init)
	{
		return;
	}

	hash->mem     = memory;
	hash->memSize = memoryBytes;

	hash->mapSize = fHash_findNextPrime(tableSize) & 0x7FFFFFFF;
	hash->nodes   = fHash_malloc(hash, sizeof(fHashNode_t *) * hash->mapSize);
	
	if (hash->nodes != NULL)
	{
		hash->init = true;
	}
	else
	{
		return;
	}

	for (u32 i = 0; i < hash->mapSize; ++i)
	{
		hash->nodes[i] = NULL;
	}
}
void fHash_initData(
	fHash_t * restrict hash,
	vptr restrict memory, u32 memoryBytes,
	const wchar ** data, u32 dataSize
)
{
	assert(hash != NULL);
	assert(memory != NULL);
	assert(memoryBytes > 0);
	assert(data != NULL);
	assert(dataSize > 0);

	if (hash->init)
	{
		return;
	}
	fHash_init(hash, memory, memoryBytes, dataSize);

	for (u32 i = 0; i < dataSize; ++i)
	{
		if (!fHash_insert(hash, data[i]))
		{
			writeProfiler("Failed on %S", data[i]);
			hash->init = false;
			return;
		}
	}
}
bool fHash_insert(fHash_t * restrict hash, const wchar * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	fHashNode_t ** pnode = &hash->nodes[fHash_hash(key, hash->mapSize)];

	while ((*pnode) != NULL)
	{
		if (wcscmp((*pnode)->key, key) == 0)
		{
			return false;
		}
		pnode = &(*pnode)->next;
	}

	fHashNode_t * node = fHash_malloc(hash, sizeof(fHashNode_t));
	if (node == NULL)
	{
		return false;
	}
	node->key  = key;
	node->next = NULL;

	*pnode = node;
	return true;
}
bool fHash_get(const fHash_t * restrict hash, const wchar * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	fHashNode_t * node = hash->nodes[fHash_hash(key, hash->mapSize)];

	while (node != NULL)
	{
		if (wcscmp(node->key, key) == 0)
		{
			return true;
		}
		node = node->next;
	}

	return false;
}
bool fHash_remove(fHash_t * restrict hash, const wchar * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	fHashNode_t ** pnode = &hash->nodes[fHash_hash(key, hash->mapSize)];
	while ((*pnode) != NULL)
	{
		if (wcscmp((*pnode)->key, key) == 0)
		{
			fHashNode_t * node = *pnode;
			*pnode = node->next;

			fHash_free(hash, node, sizeof(fHashNode_t));
			return true;
		}
		pnode = &(*pnode)->next;
	}

	return false;
}

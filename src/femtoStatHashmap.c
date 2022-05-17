#include "femtoStatHashmap.h"

#include <hashmap.h>

bool fHash_isPrime(uint32_t number)
{
	if (number <= 3)
	{
		return number > 1;
	}
	else
	{
		for (uint32_t i = 3, n = (uint32_t)sqrtf((float)number) + 1; i < n; i += 2)
		{
			if ((number % i) == 0)
			{
				return false;
			}
		}
		return true;
	}
}
uint32_t fHash_findNextPrime(uint32_t lowerBound)
{
	assert(lowerBound > 0);

	lowerBound += (lowerBound > 2) && ((lowerBound % 2) == 0);

	while (!fHash_isPrime(lowerBound))
	{
		lowerBound += 2;
	}
	return lowerBound;
}

uint32_t fHash_hash(const wchar_t * restrict key, uint32_t sz)
{
	assert(key != NULL);
	assert(sz > 0);

	uint32_t hash = 0;
	for (; (*key) != L'\0'; ++key)
	{
		hash = (uint32_t)(hash * 37 + (uint32_t)*key);
	}
	return hash % sz;
}

void * fHash_malloc(fHash_t * restrict hash, uint32_t bytes)
{
	if (hash->memSize < bytes)
	{
		return NULL;
	}

	void * mem = hash->mem;
	hash->mem = ((uint8_t *)hash->mem) + bytes;
	hash->memSize -= bytes;

	return mem;
}
void fHash_free(fHash_t * restrict hash, void * restrict mem, uint32_t bytes)
{
	void * newp = ((uint8_t *)hash->mem) - bytes;
	if (mem == newp)
	{
		hash->mem = newp;
		hash->memSize += bytes;
	}
}

void fHash_init(
	fHash_t * restrict hash,
	void * restrict memory, uint32_t memoryBytes,
	uint32_t tableSize
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
	hash->nodes   = fHash_malloc(hash, sizeof(fHNode_t *) * hash->mapSize);
	
	if (hash->nodes != NULL)
	{
		hash->init = true;
	}
	else
	{
		return;
	}

	for (uint32_t i = 0; i < hash->mapSize; ++i)
	{
		hash->nodes[i] = NULL;
	}
}
void fHash_initData(
	fHash_t * restrict hash,
	void * restrict memory, uint32_t memoryBytes,
	const wchar_t ** data, uint32_t dataSize
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

	for (uint32_t i = 0; i < dataSize; ++i)
	{
		if (!fHash_insert(hash, data[i]))
		{
			writeProfiler("Failed on %S", data[i]);
			hash->init = false;
			return;
		}
	}
}
bool fHash_insert(fHash_t * restrict hash, const wchar_t * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	fHNode_t ** pnode = &hash->nodes[fHash_hash(key, hash->mapSize)];

	while ((*pnode) != NULL)
	{
		if (wcscmp((*pnode)->key, key) == 0)
		{
			return false;
		}
		pnode = &(*pnode)->next;
	}

	fHNode_t * node = fHash_malloc(hash, sizeof(fHNode_t));
	if (node == NULL)
	{
		return false;
	}
	node->key  = key;
	node->next = NULL;

	*pnode = node;
	return true;
}
bool fHash_get(const fHash_t * restrict hash, const wchar_t * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	fHNode_t * node = hash->nodes[fHash_hash(key, hash->mapSize)];

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
bool fHash_remove(fHash_t * restrict hash, const wchar_t * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	fHNode_t ** pnode = &hash->nodes[fHash_hash(key, hash->mapSize)];
	while ((*pnode) != NULL)
	{
		if (wcscmp((*pnode)->key, key) == 0)
		{
			fHNode_t * node = *pnode;
			*pnode = node->next;

			fHash_free(hash, node, sizeof(fHNode_t));
			return true;
		}
		pnode = &(*pnode)->next;
	}

	return false;
}

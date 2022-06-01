#include "fStatHashmap.h"

#include <hashmap.h>

bool fStatHash_isPrime(u32 number)
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
u32 fStatHash_findNextPrime(u32 lowerBound)
{
	assert(lowerBound > 0);

	lowerBound += (lowerBound > 2) && ((lowerBound % 2) == 0);

	while (!fStatHash_isPrime(lowerBound))
	{
		lowerBound += 2;
	}
	return lowerBound;
}

u32 fStatHash_hash(const wchar * restrict key, u32 sz)
{
	assert(key != NULL);
	assert(sz > 0);

	usize hash = 0;
	for (; (*key) != L'\0'; ++key)
	{
		hash = (usize)(hash * (usize)37 + (usize)*key);
	}
	return (u32)(hash % (usize)sz);
}

vptr fStatHash_malloc(fStatHash_t * restrict hash, u32 bytes)
{
	if (hash->memSize < bytes)
	{
		return NULL;
	}

	vptr mem = hash->mem;
	hash->mem = ((u8 *)hash->mem) + bytes;
	hash->memSize -= bytes;

	return mem;
}
void fStatHash_free(fStatHash_t * restrict hash, vptr restrict mem, u32 bytes)
{
	vptr newp = ((u8 *)hash->mem) - bytes;
	if (mem == newp)
	{
		hash->mem = newp;
		hash->memSize += bytes;
	}
}

void fStatHash_init(
	fStatHash_t * restrict hash,
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

	hash->mapSize = fStatHash_findNextPrime(tableSize) & 0x7FFFFFFF;
	hash->nodes   = fStatHash_malloc(hash, sizeof(fHashNode_t *) * hash->mapSize);
	
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
void fStatHash_initData(
	fStatHash_t * restrict hash,
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
	fStatHash_init(hash, memory, memoryBytes, dataSize);

	for (u32 i = 0; i < dataSize; ++i)
	{
		if (!fStatHash_insert(hash, data[i]))
		{
			fProf_write("Failed on %S", data[i]);
			hash->init = false;
			return;
		}
	}
}
bool fStatHash_insert(fStatHash_t * restrict hash, const wchar * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	fHashNode_t ** restrict pnode = &hash->nodes[fStatHash_hash(key, hash->mapSize)];

	while ((*pnode) != NULL)
	{
		if (wcscmp((*pnode)->key, key) == 0)
		{
			return false;
		}
		pnode = &(*pnode)->next;
	}

	fHashNode_t * restrict node = fStatHash_malloc(hash, sizeof(fHashNode_t));
	if (node == NULL)
	{
		return false;
	}
	node->key  = key;
	node->next = NULL;

	*pnode = node;
	return true;
}
bool fStatHash_get(const fStatHash_t * restrict hash, const wchar * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	const fHashNode_t * restrict node = hash->nodes[fStatHash_hash(key, hash->mapSize)];

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
bool fStatHash_remove(fStatHash_t * restrict hash, const wchar * restrict key)
{
	assert(hash != NULL);
	assert(key != NULL);
	assert(hash->init);

	fHashNode_t ** restrict pnode = &hash->nodes[fStatHash_hash(key, hash->mapSize)];
	while ((*pnode) != NULL)
	{
		if (wcscmp((*pnode)->key, key) == 0)
		{
			fHashNode_t * restrict node = *pnode;
			*pnode = node->next;

			fStatHash_free(hash, node, sizeof(fHashNode_t));
			return true;
		}
		pnode = &(*pnode)->next;
	}

	return false;
}

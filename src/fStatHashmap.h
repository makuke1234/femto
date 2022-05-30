#ifndef FEMTO_STAT_HASHMAP_H
#define FEMTO_STAT_HASHMAP_H

#include "fCommon.h"

#pragma pack(push, 0)

typedef struct fHashNode
{
	const wchar * key;
	
	struct fHashNode * next;

} fHashNode_t;

typedef struct fStatHash
{
	vptr mem;
	u32 memSize;

	fHashNode_t ** nodes;
	u32 mapSize:31;
	u32 init:1;

} fStatHash_t;

#pragma pack(pop)

bool fStatHash_isPrime(u32 number);
u32 fStatHash_findNextPrime(u32 lowerBound);

u32 fStatHash_hash(const wchar * restrict key, u32 sz);

vptr fStatHash_malloc(fStatHash_t * restrict hash, u32 bytes);
void fStatHash_free(fStatHash_t * restrict hash, vptr restrict mem, u32 bytes);

void fStatHash_init(
	fStatHash_t * restrict hash,
	vptr restrict memory, u32 memoryBytes,
	u32 tableSize
);
void fStatHash_initData(
	fStatHash_t * restrict hash,
	vptr restrict memory, u32 memoryBytes,
	const wchar ** data, u32 dataSize
);
bool fStatHash_insert(fStatHash_t * restrict hash, const wchar * restrict key);
bool fStatHash_get(const fStatHash_t * restrict hash, const wchar * restrict key);
bool fStatHash_remove(fStatHash_t * restrict hash, const wchar * restrict key);


#endif

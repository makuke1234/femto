#ifndef FEMTO_STAT_HASHMAP_H
#define FEMTO_STAT_HASHMAP_H

#include "common.h"

#pragma pack(push, 0)

typedef struct fHashNode
{
	const wchar * key;
	
	struct fHashNode * next;

} fHashNode_t;

typedef struct fHash
{
	vptr mem;
	u32 memSize;

	fHashNode_t ** nodes;
	u32 mapSize:31;
	u32 init:1;

} fHash_t;

#pragma pack(pop)

bool fHash_isPrime(u32 number);
u32 fHash_findNextPrime(u32 lowerBound);

u32 fHash_hash(const wchar * restrict key, u32 sz);

vptr fHash_malloc(fHash_t * restrict hash, u32 bytes);
void fHash_free(fHash_t * restrict hash, vptr restrict mem, u32 bytes);

void fHash_init(
	fHash_t * restrict hash,
	vptr restrict memory, u32 memoryBytes,
	u32 tableSize
);
void fHash_initData(
	fHash_t * restrict hash,
	vptr restrict memory, u32 memoryBytes,
	const wchar ** data, u32 dataSize
);
bool fHash_insert(fHash_t * restrict hash, const wchar * restrict key);
bool fHash_get(const fHash_t * restrict hash, const wchar * restrict key);
bool fHash_remove(fHash_t * restrict hash, const wchar * restrict key);


#endif

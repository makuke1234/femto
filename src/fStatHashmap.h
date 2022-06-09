#ifndef FEMTO_STAT_HASHMAP_H
#define FEMTO_STAT_HASHMAP_H

#include "fCommon.h"

#pragma pack(push, 0)

typedef struct fHashNode
{
	const wchar * key;
	
	struct fHashNode * next;

} fHashNode_t;

#if SSIZE_MAX == INT64_MAX
	#define MAPSIZE_BITS 63
#else
	#define MAPSIZE_BITS 31
#endif

typedef struct fStatHash
{
	vptr mem;
	usize memSize;

	fHashNode_t ** nodes;
	usize mapSize:MAPSIZE_BITS;
	usize init:1;

} fStatHash_t;

#undef MAPSIZE_BITS

#pragma pack(pop)

bool fStatHash_isPrime(usize number);
usize fStatHash_findNextPrime(usize lowerBound);

usize fStatHash_hash(const wchar * restrict key, usize sz);

vptr fStatHash_malloc(fStatHash_t * restrict hash, usize bytes);
void fStatHash_free(fStatHash_t * restrict hash, vptr restrict mem, usize bytes);

void fStatHash_init(
	fStatHash_t * restrict hash,
	vptr restrict memory, usize memoryBytes,
	usize tableSize
);
void fStatHash_initData(
	fStatHash_t * restrict hash,
	vptr restrict memory, usize memoryBytes,
	const wchar ** data, usize dataSize
);
bool fStatHash_insert(fStatHash_t * restrict hash, const wchar * restrict key);
bool fStatHash_get(const fStatHash_t * restrict hash, const wchar * restrict key);
bool fStatHash_remove(fStatHash_t * restrict hash, const wchar * restrict key);


#endif

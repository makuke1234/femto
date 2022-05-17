#ifndef FEMTO_STAT_HASHMAP_H
#define FEMTO_STAT_HASHMAP_H

#include "common.h"

#pragma pack(push, 0)

typedef struct fHNode
{
	const wchar_t * key;
	
	struct fHNode * next;

} fHNode_t;

typedef struct fHash
{
	void * mem;
	uint32_t memSize;

	fHNode_t ** nodes;
	uint32_t mapSize:31;
	uint32_t init:1;

} fHash_t;

#pragma pack(pop)

bool fHash_isPrime(uint32_t number);
uint32_t fHash_findNextPrime(uint32_t lowerBound);

uint32_t fHash_hash(const wchar_t * restrict key, uint32_t sz);

void * fHash_malloc(fHash_t * restrict hash, uint32_t bytes);
void fHash_free(fHash_t * restrict hash, void * restrict mem, uint32_t bytes);

void fHash_init(
	fHash_t * restrict hash,
	void * restrict memory, uint32_t memoryBytes,
	uint32_t tableSize
);
void fHash_initData(
	fHash_t * restrict hash,
	void * restrict memory, uint32_t memoryBytes,
	const wchar_t ** data, uint32_t dataSize
);
bool fHash_insert(fHash_t * restrict hash, const wchar_t * restrict key);
bool fHash_get(const fHash_t * restrict hash, const wchar_t * restrict key);
bool fHash_remove(fHash_t * restrict hash, const wchar_t * restrict key);


#endif

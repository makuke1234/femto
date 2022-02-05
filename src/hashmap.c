#include "hashmap.h"

#include <math.h>

hashNode_t * hashNode_make(const char * key, void * value)
{
	assert(key != NULL);
	
	hashNode_t * node = malloc(sizeof(hashNode_t));
	if (node == NULL)
	{
		return NULL;
	}
	node->key   = strdup(key);
	node->value = value;
	return node;
}
void hashNode_free(hashNode_t * restrict self)
{
	assert(self != NULL);
	if (self->key != NULL)
	{
		free(self->key);
		self->key = NULL;
	}
	free(self);
}

void hashNode_recursiveFree(hashNode_t * restrict self)
{
	while (self != NULL)
	{
		hashNode_t * restrict next = self->next;
		hashNode_free(self);
		self = next;
	}
}

static inline bool hashMap_isPrime(size_t number)
{
	if (number <= 3)
	{
		return number > 1;
	}
	else
	{
		for (size_t i = 3, n = (size_t)sqrt((double)number) + 1; i < n; i += 2)
		{
			if ((number % i) == 0)
			{
				return false;
			}
		}
		return true;
	}
}

static inline size_t hashMap_findNextPrime(size_t lowerBound)
{
	if ((lowerBound > 2) && ((lowerBound % 2) == 0))
	{
		++lowerBound;
	}
	while (!hashMap_isPrime(lowerBound))
	{
		lowerBound += 2;
	}
	return lowerBound;
}

size_t hashMap_hash(const char * key, size_t mapSize)
{
	assert(key != NULL);
	assert(mapSize > 0);
	size_t hash = 0;
	for (; *key != '\0'; ++key)
	{
		hash = (size_t)(hash * (size_t)37) + (size_t)*key;
	}
	return hash % mapSize;
}

bool hashMap_init(hashMap_t * restrict self, size_t minSize)
{
	assert(self != NULL);
	assert(minSize > 0);
	self->numNodes = hashMap_findNextPrime(minSize);
	self->nodes    = malloc(self->numNodes * sizeof(hashNode_t *));
	if (self->nodes == NULL)
	{
		return false;
	}
	for (size_t i = 0; i < self->numNodes; ++i)
	{
		self->nodes[i] = NULL;
	}
	return true;
}
bool hashMap_resize(hashMap_t * restrict self, size_t minSize)
{
	assert(self != NULL);
	assert(minSize > 0);
	hashMap_t newMap;
	if (!hashMap_init(&newMap, minSize))
	{
		return false;
	}

	// Add all items from old hasmap to new one
	for (size_t i = 0; i < self->numNodes; ++i)
	{
		hashNode_t * node = self->nodes[i];
		while (node != NULL)
		{
			if (!hashMap_insert(&newMap, node->key, node->value))
			{
				hashMap_destroy(&newMap);
				return false;
			}
			node = node->next;
		}
	}

	// Destroy old hashmap
	hashMap_destroy(self);
	*self = newMap;
	return true;
}

bool hashMap_insert(hashMap_t * restrict self, const char * key, void * value)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	hashNode_t ** pnode = &self->nodes[hashMap_hash(key, self->numNodes)];

	while ((*pnode) != NULL)
	{
		if (strcmp((*pnode)->key, key) == 0)
		{
			return false;
		}
		pnode = &(*pnode)->next;
	}

	hashNode_t * node = hashNode_make(key, value);
	if (node == NULL)
	{
		return false;
	}

	*pnode = node;
	return true;
}
hashNode_t * hashMap_get(hashMap_t * restrict self, const char * key)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	hashNode_t * node = self->nodes[hashMap_hash(key, self->numNodes)];

	while (node != NULL)
	{
		if (strcmp(node->key, key) == 0)
		{
			return node;
		}
		node = node->next;
	}
	return NULL;
}
void * hashMap_remove(hashMap_t * restrict self, const char * key)
{
	assert(self != NULL);
	assert(key  != NULL);
	assert(self->nodes != NULL);

	hashNode_t ** pnode = &self->nodes[hashMap_hash(key, self->numNodes)];
	while ((*pnode) != NULL)
	{
		if (strcmp((*pnode)->key, key) == 0)
		{
			// remove node, return value
			hashNode_t * node = *pnode;
			void * value = node->value;
			*pnode = node->next;

			hashNode_free(node);
			return value;
		}
		pnode = &(*pnode)->next;
	}
	return NULL;
}


void hashMap_destroy(hashMap_t * restrict self)
{
	assert(self != NULL);
	if (self->nodes == NULL)
	{
		return;
	}

	for (size_t i = 0; i < self->numNodes; ++i)
	{
		hashNode_recursiveFree(self->nodes[i]);
	}
	free(self->nodes);
}

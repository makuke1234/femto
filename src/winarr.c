#include "winarr.h"

bool warr_init(warr_t * restrict This, usize itemSize)
{
	assert(This != NULL);
	assert(itemSize > 0);

	if (This->init)
	{
		return false;
	}

	*This = (warr_t){
		.mem      = NULL,
		.realptr  = NULL,
		.numItems = 0,
		.maxItems = 0,
		.itemSize = itemSize,
		.init     = true
	};
	return true;
}
bool warr_initSz(warr_t * restrict This, usize itemSize, usize numItems)
{
	assert(This != NULL);
	assert(itemSize > 0);

	if (!warr_init(This, itemSize))
	{
		return false;
	}
	This->init = warr_reserve(This, numItems);
	return This->init;
}
bool warr_initData(warr_t * restrict This, usize itemSize, const vptr items, usize numItems)
{
	assert(This != NULL);
	assert(itemSize > 0);
	assert(items != NULL);
	assert(numItems > 0);
	
	if (!warr_initSz(This, itemSize, numItems))
	{
		return false;
	}

	This->numItems = numItems;
	memcpy(This->mem, items, numItems * itemSize);
	return true;
}

bool warr_resize(warr_t * restrict This, usize newSize)
{
	assert(This != NULL);
	assert(This->init);

	if (newSize > This->maxItems)
	{
		bool ret = warr_reserve(This, newSize);
		This->numItems = ret ? newSize : This->numItems;
		return ret;
	}

	This->numItems = newSize;
	return true;
}
bool warr_reserve(warr_t * restrict This, usize newCap)
{
	assert(This != NULL);
	assert(This->init);
	assert(newCap > 0);

	if (newCap < This->numItems)
	{
		return false;
	}

	// Reallocating memory
	HGLOBAL newmem = NULL;
	if (This->mem != NULL)
	{
		GlobalUnlock(This->mem);
		newmem = GlobalReAlloc(This->mem, newCap * This->itemSize, GMEM_MOVEABLE);
	}
	else
	{
		newmem = GlobalAlloc(GMEM_MOVEABLE, newCap * This->itemSize);
	}

	if (newmem == NULL)
	{
		return false;
	}

	This->mem      = newmem;
	This->realptr  = GlobalLock(This->mem);
	This->maxItems = newCap;

	return true;
}
bool warr_shrinkToFit(warr_t * restrict This)
{
	assert(This != NULL);
	assert(This->init);

	return warr_reserve(This, This->numItems);
}

bool warr_pushBack(warr_t * restrict This, vptr item)
{
	assert(This != NULL);
	assert(This->init);

	if ((This->numItems >= This->maxItems) && !warr_reserve(This, (This->numItems + 1) * 2))
	{
		return false;
	}

	memcpy(warr_get(This, This->numItems), item, This->itemSize);
	++This->numItems;

	return true;
}
bool warr_removeBack(warr_t * restrict This)
{
	assert(This != NULL);
	assert(This->init);

	if (!This->numItems)
	{
		return false;
	}

	--This->numItems;
	return true;
}
vptr warr_get(const warr_t * restrict This, usize idx)
{
	assert(This != NULL);
	assert(This->init);

	return &((uint8_t *)This->realptr)[idx * This->itemSize];
}

vptr warr_data(const warr_t * restrict This)
{
	assert(This != NULL);
	assert(This->init);

	return This->realptr;
}
usize warr_size(const warr_t * restrict This)
{
	assert(This != NULL);
	assert(This->init);

	return This->numItems;
}
usize warr_cap(const warr_t * restrict This)
{
	assert(This != NULL);
	assert(This->init);

	return This->maxItems;
}
usize warr_itemSize(const warr_t * restrict This)
{
	assert(This != NULL);
	assert(This->init);

	return This->itemSize;
}
bool warr_isInit(const warr_t * restrict This)
{
	assert(This != NULL);

	return This->init;
}

HGLOBAL warr_unlock(warr_t * restrict This)
{
	assert(This != NULL);

	if (!This->init || (This->mem == NULL))
	{
		return NULL;
	}

	This->init = false;
	GlobalUnlock(This->mem);
	HGLOBAL mem = This->mem;
	This->mem = NULL;
	This->realptr = NULL;

	return mem;
}
void warr_destroy(warr_t * restrict This)
{
	assert(This != NULL);
	
	if (!This->init)
	{
		return;
	}

	if (This->mem != NULL)
	{
		GlobalUnlock(This->mem);
		GlobalFree(This->mem);
		This->mem     = NULL;
		This->realptr = NULL;
	}
	This->init = false;
}

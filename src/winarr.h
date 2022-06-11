#ifndef WIN_ARR_H
#define WIN_ARR_H

#include "fCommon.h"

typedef struct winarr
{
	HGLOBAL mem;
	vptr realptr;
	usize numItems, maxItems;
	usize itemSize;
	bool init;

} winarr_t, warr_t;

bool warr_init(warr_t * restrict This, usize itemSize);
bool warr_initSz(warr_t * restrict This, usize itemSize, usize numItems);
bool warr_initData(warr_t * restrict This, usize itemSize, const vptr items, usize numItems);

bool warr_resize(warr_t * restrict This, usize newSize);
bool warr_reserve(warr_t * restrict This, usize newCap);
bool warr_shrinkToFit(warr_t * restrict This);

bool warr_pushBack(warr_t * restrict This, vptr item);
bool warr_removeBack(warr_t * restrict This);
vptr warr_get(const warr_t * restrict This, usize idx);

vptr warr_data(const warr_t * restrict This);
usize warr_size(const warr_t * restrict This);
usize warr_cap(const warr_t * restrict This);
usize warr_itemSize(const warr_t * restrict This);
bool warr_isInit(const warr_t * restrict This);

HGLOBAL warr_unlock(warr_t * restrict This);
void warr_destroy(warr_t * restrict This);

#endif

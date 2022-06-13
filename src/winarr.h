#ifndef WIN_ARR_H
#define WIN_ARR_H

#include "fCommon.h"

// Win32 API-compatible universal array structure
typedef struct winarr
{
	HGLOBAL mem;
	vptr realptr;
	usize numItems, maxItems;
	usize itemSize;
	bool init;

} winarr_t, warr_t;

/**
 * @brief Initialises winarr
 * 
 * @param This Pointer to winarr structure
 * @param itemSize Storable item type size in bytes
 * @return true Success
 * @return false Failure
 */
bool warr_init(warr_t * restrict This, usize itemSize);
/**
 * @brief Initalises winarr with specified reserved size
 * 
 * @param This Pointer to winarr structure
 * @param itemSize Storable item type size in bytes
 * @param numItems Reserved size for array
 * @return true Success
 * @return false Failure
 */
bool warr_initSz(warr_t * restrict This, usize itemSize, usize numItems);
/**
 * @brief Initialises winarr with specified data

 * @param This Pointer to winarr structure
 * @param itemSize Storable item type size in bytes
 * @param items Array of items to be copied
 * @param numItems Number of items
 * @return true Success
 * @return false Failure
 */
bool warr_initData(warr_t * restrict This, usize itemSize, const vptr restrict items, usize numItems);

/**
 * @brief Resizes array
 * 
 * @param This Pointer to winarr structure
 * @param newSize New size of array, number of items, can be bigger or smaller
 * than current size
 * @return true Success
 * @return false Failure
 */
bool warr_resize(warr_t * restrict This, usize newSize);
/**
 * @brief Reserves memory, the new total reserved amount would be newCap number of items
 * 
 * @param This Pointer to winarr structure
 * @param newCap New reserved size of array, number of items, can be bigger than
 * number of currently used memory slots
 * @return true Success
 * @return false Failure
 */
bool warr_reserve(warr_t * restrict This, usize newCap);
/**
 * @brief Shrinks arrays reserved size to exactly size currently in use, makes the
 * memory usage of the array optimal
 * 
 * @param This Pointer to winarr structure
 * @return true Success
 * @return false Failure
 */
bool warr_shrinkToFit(warr_t * restrict This);

/**
 * @brief Pushes element to the back of the array
 * 
 * @param This Pointer to winarr structure
 * @param item Pointer to item to add to the array
 * @return true Success
 * @return false Failure
 */
bool warr_pushBack(warr_t * restrict This, const vptr restrict item);
/**
 * @brief Removes last element from array
 * 
 * @param This Pointer to winarr structure
 * @return true Success
 * @return false Failure
 */
bool warr_removeBack(warr_t * restrict This);
/**
 * @brief Requests an item from the array
 * 
 * @param This Pointer to winarr structure
 * @param idx Index of requested item
 * @return vptr Pointer to requested item, NULL on failure
 */
vptr warr_get(const warr_t * restrict This, usize idx);

/**
 * @brief Returns the pointer to the beginning of the allocated array memory
 * 
 * @param This Pointer to winarr structure
 * @return vptr Pointer to the beginning of the array memory
 */
vptr warr_data(const warr_t * restrict This);
/**
 * @brief Returns the number of currently in use memory slots
 * 
 * @param This Pointer to winarr structure
 * @return usize In use memory slots
 */
usize warr_size(const warr_t * restrict This);
/**
 * @brief Returns the number of allocated memory slots, reserved memory slots
 * 
 * @param This Pointer to winarr structure
 * @return usize Reserved memory slots
 */
usize warr_cap(const warr_t * restrict This);
/**
 * @brief Return the current element type size in bytes
 * 
 * @param This Pointer to winarr structure
 * @return usize Item size in bytes
 */
usize warr_itemSize(const warr_t * restrict This);
/**
 * @brief Checks whether the winarr structure is initialized
 * 
 * @param This Pointer to winarr structure
 * @return true Structure is initialized
 * @return false Structure is not initialized
 */
bool warr_isInit(const warr_t * restrict This);

/**
 * @brief "Detaches" Win32 allocated memory object handle from the winarr structure,
 * returns the handle itself after detaching. The detaching is carried out by "unlocking"
 * the memory.
 * 
 * @param This Pointer to winarr structure
 * @return HGLOBAL Win32 allocated memory object handle
 */
HGLOBAL warr_unlock(warr_t * restrict This);
/**
 * @brief Deallocates resources of the winarr structure. Note that it does not free
 * the pointer of the structure itself!
 * 
 * @param This Pointer to winarr structure
 */
void warr_destroy(warr_t * restrict This);

#endif

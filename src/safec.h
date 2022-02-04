#ifndef SAFEC_H
#define SAFEC_H

#include "common.h"

/**
 * @brief "Safely" duplicate string contents on the heap
 * 
 * @param str Pointer to the string character array
 * @param len String length in characters, you can use SIZE_MAX to copy
 * everything if wstr is null-terminated
 * @return char* Pointer to the newly allocated & copied string
 */
char * strdup_s(const char * str, size_t len);
/**
 * @brief "Safely" duplicate wide string contents on the heap
 * 
 * @param wstr Pointer to the wide string character array
 * @param len Wide string length in characters, you can use SIZE_MAX to copy
 * everything if wstr is null-terminated
 * @return wchar_t* Pointer to the newly allocated & copied wide string
 */
wchar_t * wcsdup_s(const wchar_t * wstr, size_t len);

#endif

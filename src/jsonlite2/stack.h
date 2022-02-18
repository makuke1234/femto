#ifndef STACK_H
#define STACK_H

#include <stdint.h>
#include <stdbool.h>

typedef struct stack
{
	void * mem;
	size_t elemSize;
	size_t len, cap;
} stack_t;

/**
 * @brief Initialise stack
 * 
 * @param self Pointer to stack_t structure
 * @param elemSize Element size
 */
void stack_init(stack_t * restrict self, size_t elemSize);
/**
 * @brief Destroy stack, free all resources allocated by it
 * 
 * @param self Pointer to stack_t structure
 */
void stack_destroy(stack_t * restrict self);

/**
 * @brief Add an item to the stack, copy it
 * 
 * @param self Pointer to stack_t structure
 * @param pitem Pointer to item to be copied
 * @return true Pushing was successful
 * @return false Failure
 */
bool stack_push(stack_t * restrict self, const void * restrict pitem);
/**
 * @brief Pops an item from the stack
 * 
 * @param self Pointer to stack_t structure
 * @return true Popping was successful
 * @return false No items to pop
 */
bool stack_pop(stack_t * restrict self);
/**
 * @brief Returns the top item from the stack
 * 
 * @param self Pointer to stack_t structure
 * @return void* Pointer to the top item on the stack
 */
void * stack_top(const stack_t * restrict self);


#endif

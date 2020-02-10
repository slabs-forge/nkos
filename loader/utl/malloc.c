/*
 * NK Loader - Utility functions
 *
 * Author       : Sebastien LAURENT
 * Date         : 02/09/2011
 *
 * memory functions
 */

#include "loader/types.h"
#include "loader/krn/mm.h"
#include "loader/utl/stdlib.h"

/*
 * Memory allocation
 */
void* malloc(size_t size) {
	return mm_malloc(size);
}

/*
 * Memory free
 */
void free(void* ptr) {
	mm_free(ptr);
}


/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 03/10/2011
 *
 * Physical Allocator
 */

#include "kernel/types.h"
#include "kernel/kernel.h"
#include "kernel/assert.h"

#include "kernel/mm/palloc.h"
#include "kernel/mm/palloc_impl.h"

#include "kernel/utl/kdebug.h"

/*
 * free pages
 */
void palloc_free(struct palloc_t* allocator,void* addr,size_t order) {
	ASSERT(allocator != 0);
	ASSERT(allocator->vptr != 0);

	ASSERT(order < MAX_ORDER);
		
	if (allocator->vptr->free) {
		allocator->vptr->free(allocator,addr,order);
	}	
}

/*
 * allocates n pages
 */
void* palloc_get(struct palloc_t* allocator,size_t order) {
	ASSERT(allocator != 0);
	ASSERT(allocator->vptr !=0);
	ASSERT(allocator->vptr !=0);

	ASSERT(order < MAX_ORDER);

	if (allocator->vptr->alloc) {
		return allocator->vptr->alloc(allocator,order);
	} else {
		return 0;
	}
}


/*
 * Initialisation
 */
struct palloc_t* palloc_init(struct palloc_t* new,struct palloc_t* old) {
	ASSERT(new!=0);
	ASSERT(new->vptr != 0)
	ASSERT(new->vptr->init != 0)

	new->vptr->init(new,old);	

	return new;
}

/*
 * Iteration sur les zones
 */
void palloc_foreach_zone(struct palloc_t* allocator,void (*callback)(void*,size_t)) {
	ASSERT(allocator!=0);
	ASSERT(allocator->vptr != 0)
	ASSERT(allocator->vptr->foreach_zone != 0)

	allocator->vptr->foreach_zone(allocator,callback);
}

/*
 * Iteration sur les zones
 */
void palloc_foreach_page(struct palloc_t* allocator,void (*callback)(void*,size_t)) {
	ASSERT(allocator!=0);
	ASSERT(allocator->vptr != 0)
	ASSERT(allocator->vptr->foreach_page != 0)

	allocator->vptr->foreach_page(allocator,callback);
}

/*
 * Debuging information
 */
void palloc_debug(struct palloc_t* allocator) {
	ASSERT(allocator != 0);
	ASSERT(allocator->vptr !=0);
	ASSERT(allocator->name !=0);

	kdebug("-- Allocator Debugging info ------------------------------------\n");
	kdebug("Allocator name: %s\n",allocator->name);
	if (allocator->vptr->debug) allocator->vptr->debug(allocator);
	kdebug("----------------------------------------------------------------\n");
}

/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 28/09/2011
 *
 * Block Allocator general
 */

#ifndef __KERNEL_BA_IMPL_H__
#define __KERNEL_BA_IMPL_H__

#include "kernel/types.h"
#include "kernel/mm/palloc.h"

void palloc_foreach_zone(struct palloc_t*,void (*)(void*,size_t));
void palloc_foreach_page(struct palloc_t*,void (*)(void*,size_t));

struct palloc_vtbl_t {
	void (*init)(struct palloc_t*,struct palloc_t*);
	void* (*alloc)(struct palloc_t*,size_t);
	void (*free)(struct palloc_t*,void*,size_t);
	void (*debug)(struct palloc_t*);
	void (*foreach_zone)(struct palloc_t*,void (*)(void*,size_t));
	void (*foreach_page)(struct palloc_t*,void (*)(void*,size_t));
};

struct palloc_t {
	const char* name;
	void *data;
	struct palloc_vtbl_t* vptr;
};


#endif


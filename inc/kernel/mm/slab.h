/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 04/10/2011
 *
 * Slab Allocator
 */

#ifndef __KERNEL_MM_SLAB_H__
#define __KERNEL_MM_SLAB_H__

#include "kernel/types.h"
#include "kernel/mm/palloc.h"

struct slab_cache_t;

void slab_init(struct palloc_t* allocator);
void slab_debug();

void* slab_allocate(struct slab_cache_t*);

struct slab_cache_t* slab_register(const char*,uint64_t,uint16_t,void (*)(void*),void (*)(void*));

#endif


/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 28/09/2011
 *
 * Block Allocator general
 */

#ifndef __KERNEL_BA_H__
#define __KERNEL_BA_H__

#include "kernel/types.h"

#define PALLOC_FAILED ((uint64_t)-1)
#define MAX_ORDER 11

struct palloc_t;

struct palloc_t* palloc_init(struct palloc_t*,struct palloc_t*);
void palloc_debug(struct palloc_t*);

void* palloc_get(struct palloc_t*,uint64_t order);
void palloc_free(struct palloc_t*,void* addr,uint64_t order);

#endif


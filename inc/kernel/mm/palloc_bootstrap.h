/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 28/09/2011
 *
 * Bootstrap allocator
 */

#ifndef __KERNEL_MM_PALLOC_BOOTSTRAP_H__
#define __KERNEL_MM_PALLOC_BOOTSTRAP_H__

#include "kernel/types.h"
#include "kernel/mm/palloc.h"

struct palloc_t* palloc_bootstrap_create(struct bootparm_t* bp);

#endif


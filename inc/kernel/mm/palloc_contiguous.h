/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 28/09/2011
 *
 * Contiguous allocator
 */

#ifndef __KERNEL_MM_PALLOC_CONTIGUOUS_H__
#define __KERNEL_MM_PALLOC_CONTIGUOUS_H__

#include "kernel/types.h"
#include "kernel/mm/palloc.h"

struct palloc_t* palloc_contiguous_create();

#endif


/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 07/10/2011
 *
 * Bitmap allocator
 */

#ifndef __KERNEL_MM_PALLOC_BITMAP_H__
#define __KERNEL_MM_PALLOC_BITMAP_H__

#include "kernel/types.h"
#include "kernel/mm/palloc.h"

struct palloc_t* palloc_bitmap_create();

#endif


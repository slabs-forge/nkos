/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 09/10/2011
 *
 * Bit scaning operation
 */

#ifndef __KERNEL_UTL_BITSCAN_H__
#define __KERNEL_UTL_BITSCAN_H__

#include "kernel/types.h"

static inline uint64_t bscan_forward(uint64_t v) {
	uint64_t r;
	asm(	"bsf %1,%0;"
		: "=r" (r)
		: "r" (v)
	);
	return r;
}

static inline uint64_t bscan_reverse(uint64_t v) {
	uint64_t r;
	asm(	"bsr %1,%0;"
		: "=r" (r)
		: "r" (v)
	);
	return r;
}

#endif


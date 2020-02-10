/*
 * Kernel - amorce
 *
 * Author       : Sebastien LAURENT
 * Date         : 20/09/2011
 *
 * far access
 */

#ifndef __KS_UTL_FAR_H__
#define __KS_UTL_FAR_H__

#include "kernel/types.h"

const char* far_strncpy(const char* dest,const char* src,size_t n);

static inline uint16_t far_readw(const void* p) {
	uint16_t r;
	asm(	"push %%ds;"
		"mov %%ax,%%si;"
		"shr $16,%%eax;"
		"mov %%ax,%%ds;"
		"lodsw;"
		"pop %%ds;"
		: "=a" (r)
		: "a" (p)
		: "esi"
	);
	return r;
}

#endif


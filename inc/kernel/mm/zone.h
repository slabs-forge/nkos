/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 14/10/2011
 *
 * Zone mamagment
 */

#ifndef __KERNEL_MM_ZONE_H__
#define __KERNEL_MM_ZONE_H__

#include "kernel/types.h"

#define MAX_ZONES 3

#define ZONE_DMA	0x01
#define ZONE_DMA32	0x10

struct zone_t {
	uint64_t bgn;
	uint64_t end;
	uint64_t flags;
	const char* name;
};

void zone_init();
void zone_debug();

#endif


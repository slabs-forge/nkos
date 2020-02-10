/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 22/09/2011
 *
 * GDT manipulation
 */

#ifndef __KS_GDT_H__
#define __KS_GT_H__

#include "kernel/types.h"

typedef uint64_t gdt_t;

struct gdt_desc_t {
	uint16_t limit;
	const void* base;
} __attribute__((packed));

static inline gdt_t gdt_segment(uint32_t base,uint32_t limit,uint8_t type) {
	gdt_t gdt = 0x0000000000000000;

	gdt |= ((uint64_t)base & 0x00FFFFFF) << 16;
	gdt |= ((uint64_t)base & 0xFF000000) << 32;
	gdt |= ((uint64_t)limit & 0x0000FFFF);
	gdt |= ((uint64_t)limit & 0x000F0000) << 32;
	gdt |= (uint64_t)type << 40;
	gdt |= 0x00C0000000000000;

	return gdt;
}

static inline void load_gdt(gdt_t* base,uint16_t size) {
	struct gdt_desc_t desc;

	desc.limit = size * 8;
	desc.base = base;
	
	asm( 
		"xor %%eax,%%eax;"
		"lea %0,%%esi;"
		"mov %%ds,%%eax;"
		"shl $4,%%eax;"
		"add %%eax,2(%%esi);"
		"lgdt %0;"
	: /* No output */
	: "m" (desc) 
	: "%eax" , "%esi"
	);
} 

#endif


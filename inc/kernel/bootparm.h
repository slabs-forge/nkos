/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 27/09/2011
 *
 * Kernel boot paramstart 
 */

#ifndef __KERNEL_BOOTPARM_H__
#define __KERNEL_BOOTPARM_H__

#include "kernel/types.h"

#define E820_ENTRIES 64

struct e820_t {
	uint64_t base;
	uint64_t size;
	uint32_t type;	
} __attribute__((packed));

struct bootparm_t {
	uint16_t relocation_segment;
	uint32_t relocation_size;
	uint16_t screen_x;
	uint16_t screen_y;
	uint32_t page_addr;
	uint64_t kernel_pla;
	uint64_t kernel_offset;
	uint64_t kernel_size;
	struct e820_t mem[E820_ENTRIES];
} __attribute__((packed));

struct bootparm_t* kbootparm(void);

#endif


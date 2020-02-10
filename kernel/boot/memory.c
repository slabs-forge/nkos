/*
 * Kernel
 *
 * Author       : Sebastien LAURENT
 * Date         : 19/09/2011
 *
 * detect memory layout
 */

#include "kernel/boot/code16gcc.h"

#include "kernel/types.h"
#include "kernel/boot/boot.h"
#include "kernel/boot/intcall.h"
#include "kernel/boot/memory.h"
#include "kernel/boot/utl/stdio.h"
#include "kernel/boot/utl/string.h"

#define SMAP 0x534d4150

int32_t mem_init(void) {
	struct regs_t regs;
	struct e820_t* mem;

	mem = get_bootparm()->mem;

	memset(&regs,0,sizeof(regs)); 
	regs.ebx = 0;
	regs.edx = SMAP;
	
	intcall(0x15,&regs,&regs);

	/*
	 * Adding bios zone as it is not fully reported by e820
	 */
	mem[0].base = 0xA0000;
	mem[0].size = 0x60000;
	mem[0].type = 2;
	
	for (uint32_t i = 1;i < E820_ENTRIES; i++) {
		regs.eax = 0xE820;
		regs.ecx = 20;
		regs.edi = (uint32_t) &(mem[i]);

		intcall(0x15,&regs,&regs);

		printf("%08x%08X : %08x%08x %08x\r\n", 
			(uint32_t)(mem[i].base >> 32),
			(uint32_t)(mem[i].base & 0xffffffff),
			(uint32_t)(mem[i].size >> 32),
			(uint32_t)(mem[i].size & 0xffffffff),
			mem->type
		);

		if (regs.ebx == 0 || (regs.eflags & EFLAGS_CARRY) == EFLAGS_CARRY) break;
	}
}
